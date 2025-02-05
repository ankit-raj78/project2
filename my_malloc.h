#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <stddef.h>

void* ts_malloc_lock(size_t size);
void ts_free_lock(void* ptr);
void* ts_malloc_nolock(size_t size);
void ts_free_nolock(void* ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();

#endif