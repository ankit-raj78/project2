#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <stddef.h>

// Add threshold for hybrid strategy
#define HYBRID_THRESHOLD 1024  // Switch between First Fit and Best Fit at 1024 bytes

void* ts_malloc_hybrid(size_t size);
void ts_free_hybrid(void* ptr);

// Existing declarations
void* ts_malloc_lock(size_t size);
void ts_free_lock(void* ptr);
void* ts_malloc_nolock(size_t size);
void ts_free_nolock(void* ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

#endif