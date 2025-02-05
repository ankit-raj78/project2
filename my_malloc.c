#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include "my_malloc.h"

typedef struct block_meta {
    size_t size;
    int free;
    struct block_meta* next;
    struct block_meta* prev;
} block_meta_t;

static block_meta_t* free_list_head = NULL;
static block_meta_t* heap_start = NULL;

static __thread block_meta_t* thread_local_free_list = NULL;
static pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sbrk_lock = PTHREAD_MUTEX_INITIALIZER;
static block_meta_t* get_block_ptr(void* ptr) {
    return (block_meta_t*)((char*)ptr - sizeof(block_meta_t));
}

static void* get_user_ptr(block_meta_t* block) {
    return (void*)((char*)block + sizeof(block_meta_t));
}
static int is_valid_block(block_meta_t* block) {
    if (!block) return 0;
    void* program_break = sbrk(0);
    return (void*)block >= (void*)heap_start && (void*)block < program_break;
}
static void remove_from_free_list(block_meta_t* block) {
    if (!block) return;
    if (block == free_list_head) {
        free_list_head = block->next;
        if (free_list_head) {
            free_list_head->prev = NULL;
        }
    } else {
        if (block->prev) {
            block->prev->next = block->next;
        }
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
    block->next = NULL;
    block->prev = NULL;
}

static void add_to_free_list(block_meta_t* block) {
    if (!block) return;
    block->free = 1;
        if (!free_list_head) {
        free_list_head = block;
        block->next = NULL;
        block->prev = NULL;
        return;
    }
block->next = free_list_head;
    block->prev = NULL;
    free_list_head->prev = block;
    free_list_head = block;
}
static block_meta_t* request_space(size_t size) {
    pthread_mutex_lock(&sbrk_lock);
    block_meta_t* block = (block_meta_t*)sbrk(0);
    void* request = sbrk(size + sizeof(block_meta_t));
    pthread_mutex_unlock(&sbrk_lock);
    if (request == (void*)-1) {
        return NULL;
    }
    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;
    if (!heap_start) {
        heap_start = block;
    }
    return block;
}

static void safe_coalesce(block_meta_t* block) {
    if (!block || !block->free) return;
    
    void* program_break = sbrk(0);
        void* next_block_addr = (char*)block + block->size + sizeof(block_meta_t);
    if (next_block_addr < program_break) {
        block_meta_t* next_block = (block_meta_t*)next_block_addr;
        if (is_valid_block(next_block) && next_block->free) {
            remove_from_free_list(next_block);
            block->size += next_block->size + sizeof(block_meta_t);
        }
    }
}

static void split_block(block_meta_t* block, size_t size) {
    if (!block) return;
        if (block->size > size + sizeof(block_meta_t) + 32) {
        block_meta_t* new_block = (block_meta_t*)((char*)block + sizeof(block_meta_t) + size);
        new_block->size = block->size - size - sizeof(block_meta_t);
        new_block->free = 1;
        new_block->next = NULL;
        new_block->prev = NULL;
        
        block->size = size;
        add_to_free_list(new_block);
    }
}

static block_meta_t* find_best_fit(size_t size) {
    block_meta_t* current = free_list_head;
    block_meta_t* best_fit = NULL;
    size_t smallest_diff = SIZE_MAX;
    
    while (current != NULL) {
        if (current->free && current->size >= size) {
            size_t diff = current->size - size;
            if (diff < smallest_diff) {
                smallest_diff = diff;
                best_fit = current;
                if (diff == 0) break;
            }
        }
        current = current->next;
    }
    return best_fit;
}
void* ts_malloc_lock(size_t size) {
    if (size == 0) return NULL;
    
    pthread_mutex_lock(&global_malloc_lock);
    
    block_meta_t* best_fit = find_best_fit(size);
    
    if (!best_fit) {
        best_fit = request_space(size);
        if (!best_fit) {
            pthread_mutex_unlock(&global_malloc_lock);
            return NULL;
        }
    } else {
        remove_from_free_list(best_fit);
        split_block(best_fit, size);
    }
    best_fit->free = 0;
    void* result = get_user_ptr(best_fit);
    pthread_mutex_unlock(&global_malloc_lock);
    return result;
}
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
void* ts_malloc_nolock(size_t size) {
    if (size == 0) return NULL;
    
    block_meta_t* block = request_space(size);  // Only lock around sbrk
    if (!block) return NULL;
    
    return get_user_ptr(block);
}

void ts_free_nolock(void* ptr) {
    if (!ptr) return;   
    block_meta_t* block = get_block_ptr(ptr);
    if (!is_valid_block(block)) return;
        block->free = 1;
}

unsigned long get_data_segment_size() {
    if (!heap_start) return 0;
    void* program_break = sbrk(0);
    return (unsigned long)((char*)program_break - (char*)heap_start);
}
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