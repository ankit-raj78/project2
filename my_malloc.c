#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "my_malloc.h"

typedef struct block_meta {
    size_t size;
    int free;
    struct block_meta* next;
    struct block_meta* prev;
} block_meta_t;

static block_meta_t* free_list_head = NULL;
static block_meta_t* heap_start = NULL;

static block_meta_t* get_block_ptr(void* ptr) {
    return (block_meta_t*)((char*)ptr - sizeof(block_meta_t));
}

static void* get_user_ptr(block_meta_t* block) {
    return (void*)((char*)block + sizeof(block_meta_t));
}

static void remove_from_free_list(block_meta_t* block) {
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
    block->free = 1;
    if (!free_list_head) {
        free_list_head = block;
        return;
    }
    block->next = free_list_head;
    block->prev = NULL;
    free_list_head->prev = block;
    free_list_head = block;
}
static block_meta_t* request_space(size_t size) {
    void* block_addr = sbrk(0);
    block_meta_t* block = (block_meta_t*)block_addr;
    void* request = sbrk(size + sizeof(block_meta_t));
    
    if (request == (void*)-1) {
        return NULL;
    }

    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;
    
    if (heap_start == NULL) {
        heap_start = block;
    }
    
    return block;
}
static void coalesce(block_meta_t* block) {
    char* next_block_addr = (char*)block + block->size + sizeof(block_meta_t);
    block_meta_t* next_phys_block = (block_meta_t*)next_block_addr;
    void* program_break = sbrk(0);
        if (next_block_addr < (char*)program_break && next_phys_block->free) {
        remove_from_free_list(next_phys_block);
        block->size += next_phys_block->size + sizeof(block_meta_t);
    }
}
static void split_block(block_meta_t* block, size_t size) {
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

void* ff_malloc(size_t size) {
    if (size == 0) return NULL;

    block_meta_t* current = free_list_head;
    block_meta_t* best_fit = NULL;

    while (current != NULL) {
        if (current->free && current->size >= size) {
            best_fit = current;
            break;
        }
        current = current->next;
    }

    if (best_fit == NULL) {
        best_fit = request_space(size);
        if (best_fit == NULL) return NULL;
    } else {
        remove_from_free_list(best_fit);
        split_block(best_fit, size);
    }

    best_fit->free = 0;
    return get_user_ptr(best_fit);
}
void ff_free(void* ptr) {
    if (ptr == NULL) return;

    block_meta_t* block = get_block_ptr(ptr);
    block->free = 1;
    add_to_free_list(block);
    coalesce(block);
}
void* bf_malloc(size_t size) {
    if (size == 0) return NULL;

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
    if (best_fit == NULL) {
        best_fit = request_space(size);
        if (best_fit == NULL) return NULL;
    } else {
        remove_from_free_list(best_fit);
        split_block(best_fit, size);
    }

    best_fit->free = 0;
    return get_user_ptr(best_fit);
}

void bf_free(void* ptr) {
    if (ptr == NULL) return;

    block_meta_t* block = get_block_ptr(ptr);
    block->free = 1;
    add_to_free_list(block);
    coalesce(block);
}
unsigned long get_data_segment_size() {
    if (heap_start == NULL) {
        return 0;
    }
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