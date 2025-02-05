# ECE650 Project 2: Thread-Safe Memory Allocator

## Overview
This project implements a thread-safe memory allocator with two different versions:
- Lock-based implementation
- Non-locking implementation

## Implementation Details

### Lock-Based Version
- Uses mutex locks to ensure thread safety
- Functions:
    - `ts_malloc_lock()`: Thread-safe malloc with locking
    - `ts_free_lock()`: Thread-safe free with locking
- Features best-fit allocation strategy
- Implements memory coalescing
- Uses a free list for memory management

### Non-Locking Version
- Utilizes thread-local storage
- Functions:
    - `ts_malloc_nolock()`: Thread-safe malloc without locks
    - `ts_free_nolock()`: Thread-safe free without locks
- Simpler implementation but may be less memory efficient

### Memory Management
- Uses metadata blocks to track allocations
- Implements block splitting for better memory utilization
- Maintains linked list of free blocks
- Supports coalescing of adjacent free blocks

### Performance Tracking
- `get_data_segment_size()`: Reports total allocated segment size
- `get_data_segment_free_space_size()`: Reports available free space

## Testing
- Multiple thread test cases included
- Tests for correctness and thread safety
- Performance measurement capabilities
- Validates against memory overlaps and leaks

PROJECT2:  
![PROJECT2 Diagram](images/PROJECT2.png)

SQD:  
![Sequence Diagram](images/sqd.png)

## Compilation & Usage
```bash
make clean
make all
cd thread_tests
make clean
make all
```

## Design Decisions
1. Best-fit allocation strategy for efficient memory use
2. Block splitting to minimize fragmentation
3. Immediate coalescing of adjacent free blocks
4. Thread-safe design using mutex locks
5. Separate free lists for better performance

## Author
Amogh Raghunath