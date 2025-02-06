#define _DEFAULT_SOURCE   /* Required for sbrk() function */
#include <unistd.h>      /* For sbrk() */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>     /* For thread synchronization primitives */
#include "my_malloc.h"

/* 
 * Block metadata structure
 * Contains all necessary information to manage a memory block
 * Size: Implementation dependent, but typically 24 bytes on 64-bit systems
 */
typedef struct block_meta {
    size_t size;              /* Size of the data portion (not including metadata) */
    int free;                 /* 1 if block is free, 0 if allocated */
    struct block_meta* next;  /* Points to next block in free list */
    struct block_meta* prev;  /* Points to previous block in free list */
} block_meta_t;

/* Global state management */
static block_meta_t* free_list_head = NULL;  /* Head of the free blocks list */
static block_meta_t* heap_start = NULL;      /* First block in the heap */

/* Thread-specific management */
static __thread block_meta_t* thread_local_free_list = NULL;  /* Thread-local free list */
static pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;  /* Main allocation lock */
static pthread_mutex_t sbrk_lock = PTHREAD_MUTEX_INITIALIZER;          /* Heap growth lock */

/*
 * Converts a user pointer to its corresponding metadata block
 * Parameters:
 *   ptr: Pointer to user data region
 * Returns:
 *   Pointer to the block's metadata
 */
static block_meta_t* get_block_ptr(void* ptr) {
    return (block_meta_t*)((char*)ptr - sizeof(block_meta_t));
}

/*
 * Converts a metadata block pointer to its user data region
 * Parameters:
 *   block: Pointer to block metadata
 * Returns:
 *   Pointer to user data region
 */
static void* get_user_ptr(block_meta_t* block) {
    return (void*)((char*)block + sizeof(block_meta_t));
}

/*
 * Validates if a block pointer is within the heap bounds
 * This is crucial for memory safety and preventing heap corruption
 * Parameters:
 *   block: Block to validate
 * Returns:
 *   1 if valid, 0 if invalid
 */
static int is_valid_block(block_meta_t* block) {
    if (!block) return 0;
    void* program_break = sbrk(0);  /* Get current program break */
    /* Check if block is within heap bounds */
    return (void*)block >= (void*)heap_start && (void*)block < program_break;
}

/*
 * Removes a block from the free list
 * Handles all edge cases of list manipulation
 * Parameters:
 *   block: Block to remove from free list
 */
static void remove_from_free_list(block_meta_t* block) {
    if (!block) return;
    
    /* Handle removal if block is head of list */
    if (block == free_list_head) {
        free_list_head = block->next;
        if (free_list_head) {
            free_list_head->prev = NULL;
        }
    } else {
        /* Handle removal from middle/end of list */
        if (block->prev) {
            block->prev->next = block->next;
        }
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
    /* Clear block's list pointers */
    block->next = NULL;
    block->prev = NULL;
}

/*
 * Adds a block to the free list
 * Uses a LIFO (stack) approach for simplicity and efficiency
 * Parameters:
 *   block: Block to add to free list
 */
static void add_to_free_list(block_meta_t* block) {
    if (!block) return;
    block->free = 1;
    
    /* If list is empty, block becomes the only element */
    if (!free_list_head) {
        free_list_head = block;
        block->next = NULL;
        block->prev = NULL;
        return;
    }
    
    /* Add block to front of list */
    block->next = free_list_head;
    block->prev = NULL;
    free_list_head->prev = block;
    free_list_head = block;
}

/*
 * Requests new space from the operating system
 * Uses sbrk() with thread-safe locking
 * Parameters:
 *   size: Amount of space needed for user data
 * Returns:
 *   Pointer to new block, or NULL if allocation failed
 */
static block_meta_t* request_space(size_t size) {
    pthread_mutex_lock(&sbrk_lock);  /* Ensure thread-safe heap growth */
    
    block_meta_t* block = (block_meta_t*)sbrk(0);  /* Get current break */
    void* request = sbrk(size + sizeof(block_meta_t));  /* Request new space */
    
    pthread_mutex_unlock(&sbrk_lock);
    
    if (request == (void*)-1) {  /* Check for allocation failure , this is a special
                                    value returned by system call functions like sbrk*/
        return NULL;
    }
    
    /* Initialize new block */
    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;
    
    /* Update heap start if this is first allocation */
    if (!heap_start) {
        heap_start = block;
    }
    
    return block;
}

/*
 * Safely merges adjacent free blocks to combat fragmentation
 * Parameters:
 *   block: Starting block for coalescing
 */
static void safe_coalesce(block_meta_t* block) {
    if (!block || !block->free) return;
    
    void* program_break = sbrk(0);
    /* Calculate address of next potential block */
    void* next_block_addr = (char*)block + block->size + sizeof(block_meta_t);
    
    if (next_block_addr < program_break) {
        block_meta_t* next_block = (block_meta_t*)next_block_addr;
        /* If next block is valid and free, merge them */
        if (is_valid_block(next_block) && next_block->free) {
            remove_from_free_list(next_block);
            block->size += next_block->size + sizeof(block_meta_t);
        }
    }
}

/*
 * Splits a block if it's significantly larger than needed
 * Uses 32-byte threshold to prevent excessive fragmentation
 * 32 is an arbitrary value, but it's a reasonable trade-off
 * for alligned memory architecture 
 * Parameters:
 *   block: Block to potentially split
 *   size: Size needed for allocation
 */
static void split_block(block_meta_t* block, size_t size) {
    if (!block) return;
    
    /* Only split if remaining size would be useful */
    if (block->size > size + sizeof(block_meta_t) + 32) {
        /* Create new block from excess space */
        block_meta_t* new_block = (block_meta_t*)((char*)block + sizeof(block_meta_t) + size);
        new_block->size = block->size - size - sizeof(block_meta_t);
        new_block->free = 1;
        new_block->next = NULL;
        new_block->prev = NULL;
        
        /* Update original block */
        block->size = size;
        add_to_free_list(new_block);
    }
}

/*
 * Implements best-fit allocation strategy
 * Searches entire free list to find smallest sufficient block
 * Parameters:
 *   size: Size needed for allocation
 * Returns:
 *   Best fitting block, or NULL if none found
 */
static block_meta_t* find_best_fit(size_t size) {
    block_meta_t* current = free_list_head;
    block_meta_t* best_fit = NULL;
    size_t smallest_diff = SIZE_MAX;
    
    /* Search entire free list for best fit */
    while (current != NULL) {
        if (current->free && current->size >= size) {
            size_t diff = current->size - size;
            if (diff < smallest_diff) {
                smallest_diff = diff;
                best_fit = current;
                if (diff == 0) break;  /* Perfect fit found */
            }
        }
        current = current->next;
    }
    return best_fit;
}

/*
 * Thread-safe malloc implementation
 * Parameters:
 *   size: Number of bytes to allocate
 * Returns:
 *   Pointer to allocated memory, or NULL on failure
 */
void* ts_malloc_lock(size_t size) {
    if (size == 0) return NULL;
    
    pthread_mutex_lock(&global_malloc_lock);
    
    /* Try to find a suitable free block */
    block_meta_t* best_fit = find_best_fit(size);
    
    if (!best_fit) {
        /* No suitable block found, request new space */
        best_fit = request_space(size);
        if (!best_fit) {
            pthread_mutex_unlock(&global_malloc_lock);
            return NULL;
        }
    } else {
        /* Suitable block found, prepare it for use */
        remove_from_free_list(best_fit);
        split_block(best_fit, size);
    }
    
    best_fit->free = 0;
    void* result = get_user_ptr(best_fit);
    
    pthread_mutex_unlock(&global_malloc_lock);
    return result;
}

/*
 * Thread-safe free implementation
 * Parameters:
 *   ptr: Pointer to memory to free
 */
void ts_free_lock(void* ptr) {
    if (!ptr) return;
    
    pthread_mutex_lock(&global_malloc_lock);
    
    block_meta_t* block = get_block_ptr(ptr);
    if (is_valid_block(block)) {
        add_to_free_list(block);
        safe_coalesce(block);
    }
    
    pthread_mutex_unlock(&global_malloc_lock);
}

/*
 * Non-thread-safe malloc implementation
 * Simpler and faster than thread-safe version
 * Parameters:
 *   size: Number of bytes to allocate
 * Returns:
 *   Pointer to allocated memory, or NULL on failure
 */
void* ts_malloc_nolock(size_t size) {
    if (size == 0) return NULL;
    
    /* Simply request new space every time */
    block_meta_t* block = request_space(size);
    if (!block) return NULL;
    
    return get_user_ptr(block);
}

/*
 * Non-thread-safe free implementation
 * Parameters:
 *   ptr: Pointer to memory to free
 */
void ts_free_nolock(void* ptr) {
    if (!ptr) return;
    
    block_meta_t* block = get_block_ptr(ptr);
    if (!is_valid_block(block)) return;
    
    block->free = 1;  /* Simply mark as free */
}

/*
 * Returns total size of allocated heap
 * Returns:
 *   Total size in bytes
 */
unsigned long get_data_segment_size() {
    if (!heap_start) return 0;
    void* program_break = sbrk(0);
    return (unsigned long)((char*)program_break - (char*)heap_start);
}

/*
 * Returns total size of free space
 * Returns:
 *   Total free space in bytes
 */
unsigned long get_data_segment_free_space_size() {
    block_meta_t* current = free_list_head;
    unsigned long total_free = 0;
    
    while (current != NULL) {
        if (current->free) {
            total_free += current->size + sizeof(block_meta_t);
        }
        current = current->next;
    }
    return total_free;
}