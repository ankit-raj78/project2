#include <criterion/criterion.h>
#include <criterion/hooks.h>
#include <criterion/logging.h>
#include "my_malloc.h"
#include <time.h>
#include <stdlib.h>

#define SMALL_ALLOC 512    // Below threshold
#define LARGE_ALLOC 2048   // Above threshold
#define NUM_ITERATIONS 1000
#define HYBRID_THRESHOLD 1024  // Switching threshold

typedef struct {
    double small_alloc_time;
    double large_alloc_time;
    double mixed_alloc_time;
    size_t fragmentation;
    size_t total_memory;
    size_t free_memory;
} perf_metrics_t;

static perf_metrics_t metrics;

// Setup and teardown
void suite_setup(void) {
    memset(&metrics, 0, sizeof(metrics));
}

Test(hybrid_malloc, small_allocations, .init = suite_setup) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    void* ptrs[NUM_ITERATIONS];
    
    // Test small allocations (should use First Fit)
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        ptrs[i] = ts_malloc_hybrid(SMALL_ALLOC);
        cr_assert_not_null(ptrs[i], "Small allocation failed");
    }
    
    // Cleanup
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        ts_free_hybrid(ptrs[i]);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    metrics.small_alloc_time = (end.tv_sec - start.tv_sec) + 
                              (end.tv_nsec - start.tv_nsec) / 1e9;
    
    cr_log_info("Small Allocations (First Fit) Time: %.4f seconds\n", 
                metrics.small_alloc_time);
}

Test(hybrid_malloc, large_allocations) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    void* ptrs[NUM_ITERATIONS];
    
    // Test large allocations (should use Best Fit)
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        ptrs[i] = ts_malloc_hybrid(LARGE_ALLOC);
        cr_assert_not_null(ptrs[i], "Large allocation failed");
    }
    
    // Cleanup
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        ts_free_hybrid(ptrs[i]);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    metrics.large_alloc_time = (end.tv_sec - start.tv_sec) + 
                              (end.tv_nsec - start.tv_nsec) / 1e9;
    
    cr_log_info("Large Allocations (Best Fit) Time: %.4f seconds\n", 
                metrics.large_alloc_time);
}

Test(hybrid_malloc, mixed_size_allocations) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    void* ptrs[NUM_ITERATIONS * 2];
    size_t sizes[NUM_ITERATIONS * 2];
    
    // Create mixed allocation pattern
    for(int i = 0; i < NUM_ITERATIONS * 2; i++) {
        sizes[i] = (i % 2) ? LARGE_ALLOC : SMALL_ALLOC;
        ptrs[i] = ts_malloc_hybrid(sizes[i]);
        cr_assert_not_null(ptrs[i], "Mixed allocation failed");
    }
    
    // Random deallocation to create fragmentation
    for(int i = NUM_ITERATIONS * 2 - 1; i >= 0; i--) {
        ts_free_hybrid(ptrs[i]);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    metrics.mixed_alloc_time = (end.tv_sec - start.tv_sec) + 
                              (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Measure fragmentation
    metrics.total_memory = get_data_segment_size();
    metrics.free_memory = get_data_segment_free_space_size();
    metrics.fragmentation = metrics.total_memory - metrics.free_memory;
    
    cr_log_info("\nMixed Allocation Performance Metrics:\n");
    cr_log_info("Execution Time: %.4f seconds\n", metrics.mixed_alloc_time);
    cr_log_info("Total Memory: %zu bytes\n", metrics.total_memory);
    cr_log_info("Free Memory: %zu bytes\n", metrics.free_memory);
    cr_log_info("Fragmentation: %zu bytes\n", metrics.fragmentation);
}

Test(hybrid_malloc, fragmentation_analysis) {
    void* ptrs[100];
    size_t sizes[100];
    
    // Create fragmented heap state
    for(int i = 0; i < 100; i++) {
        sizes[i] = (rand() % (LARGE_ALLOC - SMALL_ALLOC + 1)) + SMALL_ALLOC;
        ptrs[i] = ts_malloc_hybrid(sizes[i]);
        cr_assert_not_null(ptrs[i]);
    }
    
    // Free every other block to create fragmentation
    for(int i = 0; i < 100; i += 2) {
        ts_free_hybrid(ptrs[i]);
    }
    
    // Try to allocate in gaps
    void* test_ptr = ts_malloc_hybrid(SMALL_ALLOC);
    cr_assert_not_null(test_ptr, "Failed to allocate in fragmented heap");
    
    // Cleanup
    ts_free_hybrid(test_ptr);
    for(int i = 1; i < 100; i += 2) {
        ts_free_hybrid(ptrs[i]);
    }
    
    size_t final_fragmentation = get_data_segment_size() - 
                                get_data_segment_free_space_size();
    cr_log_info("Final Fragmentation: %zu bytes\n", final_fragmentation);
}
