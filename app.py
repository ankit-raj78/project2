# # import streamlit as st
# # import plotly.express as px
# # import pandas as pd
# # import numpy as np

# # # Define thresholds and data
# # thresholds = [32, 256, 1024, 2048]
# # data = {
# #     'small_alloc': {
# #         'exec_time': [0.411901, 0.116640, 0.104019, 0.103144],
# #         'frag': [0.023914, 0.007642, 0.000840, 0.000980],
# #         'data_segment': [3642432, 3781024, 4801152, 4996000],
# #         'free_space': [87104, 28896, 4032, 4896]
# #     },
# #     'equal_size': {
# #         'exec_time': [1.177422, 0.924178, 0.918667, 0.935039],
# #         'frag': [0.450000, 0.450000, 0.450000, 0.450000]
# #     },
# #     'large_range': {
# #         'exec_time': [75.189382, 44.420004, 20.679957, 6.985599],
# #         'frag': [0.063248, 0.057225, 0.048910, 0.043617]
# #     }
# # }

# # st.title("Performance Analysis of Custom Malloc Implementation")
# # st.write("""Interactive visualization of the execution time, fragmentation, and memory usage of
# # your custom malloc implementation using different threshold sizes.""")

# # # Execution Time Plot
# # time_df = pd.DataFrame({
# #     'Threshold Size': thresholds * 3,
# #     'Execution Time (s)': data['small_alloc']['exec_time'] + 
# #                           data['equal_size']['exec_time'] + 
# #                           data['large_range']['exec_time'],
# #     'Allocation Type': ['Small Range'] * 4 + ['Equal Size'] * 4 + ['Large Range'] * 4
# # })
# # fig_time = px.line(time_df, x='Threshold Size', y='Execution Time (s)', color='Allocation Type',
# #                     markers=True, log_x=True, log_y=True,
# #                     title="Execution Time vs Threshold Size")
# # st.plotly_chart(fig_time)

# # # Fragmentation Plot
# # frag_df = pd.DataFrame({
# #     'Threshold Size': thresholds * 3,
# #     'Fragmentation Ratio': data['small_alloc']['frag'] + 
# #                            data['equal_size']['frag'] + 
# #                            data['large_range']['frag'],
# #     'Allocation Type': ['Small Range'] * 4 + ['Equal Size'] * 4 + ['Large Range'] * 4
# # })
# # fig_frag = px.line(frag_df, x='Threshold Size', y='Fragmentation Ratio', color='Allocation Type',
# #                     markers=True, log_x=True,
# #                     title="Fragmentation vs Threshold Size")
# # st.plotly_chart(fig_frag)

# # # Memory Usage Plot (Small Allocations)
# # memory_df = pd.DataFrame({
# #     'Threshold Size': thresholds * 2,
# #     'Memory Size (MB)': [x / 1e6 for x in data['small_alloc']['data_segment']] + 
# #                          [x / 1e6 for x in data['small_alloc']['free_space']],
# #     'Type': ['Total Segment'] * 4 + ['Free Space'] * 4
# # })
# # fig_memory = px.bar(memory_df, x='Threshold Size', y='Memory Size (MB)', color='Type',
# #                      barmode='group', title="Memory Usage for Small Allocations")
# # st.plotly_chart(fig_memory)

# # # Performance Summary
# # improvements = {
# #     'Small Range': data['small_alloc']['exec_time'][0] / data['small_alloc']['exec_time'][-1],
# #     'Equal Size': data['equal_size']['exec_time'][0] / data['equal_size']['exec_time'][-1],
# #     'Large Range': data['large_range']['exec_time'][0] / data['large_range']['exec_time'][-1]
# # }
# # improve_df = pd.DataFrame(list(improvements.items()), columns=['Allocation Type', 'Speedup Factor'])
# # fig_improve = px.bar(improve_df, x='Allocation Type', y='Speedup Factor',
# #                       text_auto='.2f', title="Performance Improvement Summary")
# # st.plotly_chart(fig_improve)
# import streamlit as st
# import plotly.express as px
# import pandas as pd
# import numpy as np

# # Define thresholds and data
# thresholds = [32, 256, 1024, 2048]
# data = {
#     'small_alloc': {
#         'exec_time': [0.411901, 0.116640, 0.104019, 0.103144],
#         'frag': [0.023914, 0.007642, 0.000840, 0.000980],
#         'data_segment': [3642432, 3781024, 4801152, 4996000],
#         'free_space': [87104, 28896, 4032, 4896]
#     },
#     'equal_size': {
#         'exec_time': [1.177422, 0.924178, 0.918667, 0.935039],
#         'frag': [0.450000, 0.450000, 0.450000, 0.450000]
#     },
#     'large_range': {
#         'exec_time': [75.189382, 44.420004, 20.679957, 6.985599],
#         'frag': [0.063248, 0.057225, 0.048910, 0.043617]
#     }
# }

# st.title("Performance Analysis of Custom Malloc Implementation")
# st.write("""
# ## Introduction
# This report provides a comprehensive analysis of the custom memory allocator implementation, including execution time, fragmentation, and memory usage.

# ## Technical Implementation
# The memory allocator is implemented using the Best Fit algorithm and supports both thread-safe and non-thread-safe versions:
# - **Thread-safe version (`ts_malloc_lock`)**: Uses a global lock to ensure atomic memory operations.
# - **Non-thread-safe version (`ts_malloc_nolock`)**: Provides faster performance but lacks concurrency control.

# ### Key Features
# - **Memory Coalescing**: Adjacent free blocks are merged to reduce fragmentation.
# - **Splitting Large Blocks**: Ensures efficient memory allocation by splitting large free blocks.
# - **Thread-local Storage (TLS)**: Used in the non-locking version to improve performance.

# ## Performance Metrics
# ### Execution Time
# """)

# # Execution Time Plot
# time_df = pd.DataFrame({
#     'Threshold Size': thresholds * 3,
#     'Execution Time (s)': data['small_alloc']['exec_time'] + 
#                           data['equal_size']['exec_time'] + 
#                           data['large_range']['exec_time'],
#     'Allocation Type': ['Small Range'] * 4 + ['Equal Size'] * 4 + ['Large Range'] * 4
# })
# fig_time = px.line(time_df, x='Threshold Size', y='Execution Time (s)', color='Allocation Type',
#                     markers=True, log_x=True, log_y=True,
#                     title="Execution Time vs Threshold Size")
# st.plotly_chart(fig_time)

# st.write("""
# ### Fragmentation Analysis
# Memory fragmentation is measured as the ratio of free space to total allocated memory.
# """)

# # Fragmentation Plot
# frag_df = pd.DataFrame({
#     'Threshold Size': thresholds * 3,
#     'Fragmentation Ratio': data['small_alloc']['frag'] + 
#                            data['equal_size']['frag'] + 
#                            data['large_range']['frag'],
#     'Allocation Type': ['Small Range'] * 4 + ['Equal Size'] * 4 + ['Large Range'] * 4
# })
# fig_frag = px.line(frag_df, x='Threshold Size', y='Fragmentation Ratio', color='Allocation Type',
#                     markers=True, log_x=True,
#                     title="Fragmentation vs Threshold Size")
# st.plotly_chart(fig_frag)

# st.write("""
# ### Memory Usage Analysis
# The following graph shows the total memory segment and free space for small allocations.
# """)

# # Memory Usage Plot (Small Allocations)
# memory_df = pd.DataFrame({
#     'Threshold Size': thresholds * 2,
#     'Memory Size (MB)': [x / 1e6 for x in data['small_alloc']['data_segment']] + 
#                          [x / 1e6 for x in data['small_alloc']['free_space']],
#     'Type': ['Total Segment'] * 4 + ['Free Space'] * 4
# })
# fig_memory = px.bar(memory_df, x='Threshold Size', y='Memory Size (MB)', color='Type',
#                      barmode='group', title="Memory Usage for Small Allocations")
# st.plotly_chart(fig_memory)

# st.write("""
# ### Performance Summary
# The speedup factor shows the improvement in execution time when increasing the threshold size.
# """)

# # Performance Summary
# improvements = {
#     'Small Range': data['small_alloc']['exec_time'][0] / data['small_alloc']['exec_time'][-1],
#     'Equal Size': data['equal_size']['exec_time'][0] / data['equal_size']['exec_time'][-1],
#     'Large Range': data['large_range']['exec_time'][0] / data['large_range']['exec_time'][-1]
# }
# improve_df = pd.DataFrame(list(improvements.items()), columns=['Allocation Type', 'Speedup Factor'])
# fig_improve = px.bar(improve_df, x='Allocation Type', y='Speedup Factor',
#                       text_auto='.2f', title="Performance Improvement Summary")
# st.plotly_chart(fig_improve)

# st.write("""
# ## Conclusion
# - The thread-safe version provides robustness for multi-threaded applications at the cost of performance.
# - The non-thread-safe version is significantly faster but lacks concurrency control.
# - Increasing the threshold size significantly reduces execution time and fragmentation, but beyond 1024B, improvements are marginal.

# This report demonstrates that the best balance of speed and efficiency is achieved with the **1024-byte threshold** for general use cases.
# """)
import streamlit as st
import plotly.express as px
import pandas as pd
import numpy as np

# Define thresholds and data
thresholds = [32, 256, 1024, 2048]
data = {
    'small_alloc': {
        'exec_time': [0.411901, 0.116640, 0.104019, 0.103144],
        'frag': [0.023914, 0.007642, 0.000840, 0.000980],
        'data_segment': [3642432, 3781024, 4801152, 4996000],
        'free_space': [87104, 28896, 4032, 4896]
    },
    'equal_size': {
        'exec_time': [1.177422, 0.924178, 0.918667, 0.935039],
        'frag': [0.450000, 0.450000, 0.450000, 0.450000]
    },
    'large_range': {
        'exec_time': [75.189382, 44.420004, 20.679957, 6.985599],
        'frag': [0.063248, 0.057225, 0.048910, 0.043617]
    }
}

st.title("Performance Analysis of Custom Malloc Implementation")
st.write("""
## Introduction
This report provides a comprehensive analysis of the custom memory allocator implementation, including execution time, fragmentation, and memory usage.

## Technical Implementation
The memory allocator is implemented using the Best Fit algorithm and supports both thread-safe and non-thread-safe versions:
- **Thread-safe version (`ts_malloc_lock`)**: Uses a global lock to ensure atomic memory operations.
- **Non-thread-safe version (`ts_malloc_nolock`)**: Provides faster performance but lacks concurrency control.

### Key Features
- **Memory Coalescing**: Adjacent free blocks are merged to reduce fragmentation.
- **Splitting Large Blocks**: Ensures efficient memory allocation by splitting large free blocks.
- **Thread-local Storage (TLS)**: Used in the non-locking version to improve performance.

## Potential Use Cases
- **High-Performance Computing (HPC)**: Efficient memory management is critical for HPC applications that demand optimized allocation strategies.
- **Embedded Systems**: Limited memory environments require fragmentation-aware allocation.
- **Concurrent Multi-threaded Applications**: Ensuring thread-safe allocations for applications that rely on dynamic memory.
- **Database Systems**: Optimizing memory allocation for caching and transaction processing.
- **Operating Systems**: Custom memory allocators can improve OS-level memory management strategies.

## Future Enhancements
- Implementing a hybrid allocation strategy that dynamically switches between Best Fit and First Fit.
- Adding NUMA-aware memory allocation for optimizing access patterns in multi-core environments.
- Exploring hardware acceleration techniques to speed up memory allocation operations.
- Introducing adaptive fragmentation mitigation techniques based on workload patterns.

## Performance Metrics
### Execution Time
""")

# Execution Time Plot
time_df = pd.DataFrame({
    'Threshold Size': thresholds * 3,
    'Execution Time (s)': data['small_alloc']['exec_time'] + 
                          data['equal_size']['exec_time'] + 
                          data['large_range']['exec_time'],
    'Allocation Type': ['Small Range'] * 4 + ['Equal Size'] * 4 + ['Large Range'] * 4
})
fig_time = px.line(time_df, x='Threshold Size', y='Execution Time (s)', color='Allocation Type',
                    markers=True, log_x=True, log_y=True,
                    title="Execution Time vs Threshold Size")
st.plotly_chart(fig_time)

st.write("""
### Fragmentation Analysis
Memory fragmentation is measured as the ratio of free space to total allocated memory.
""")

# Fragmentation Plot
frag_df = pd.DataFrame({
    'Threshold Size': thresholds * 3,
    'Fragmentation Ratio': data['small_alloc']['frag'] + 
                           data['equal_size']['frag'] + 
                           data['large_range']['frag'],
    'Allocation Type': ['Small Range'] * 4 + ['Equal Size'] * 4 + ['Large Range'] * 4
})
fig_frag = px.line(frag_df, x='Threshold Size', y='Fragmentation Ratio', color='Allocation Type',
                    markers=True, log_x=True,
                    title="Fragmentation vs Threshold Size")
st.plotly_chart(fig_frag)

st.write("""
### Memory Usage Analysis
The following graph shows the total memory segment and free space for small allocations.
""")

# Memory Usage Plot (Small Allocations)
memory_df = pd.DataFrame({
    'Threshold Size': thresholds * 2,
    'Memory Size (MB)': [x / 1e6 for x in data['small_alloc']['data_segment']] + 
                         [x / 1e6 for x in data['small_alloc']['free_space']],
    'Type': ['Total Segment'] * 4 + ['Free Space'] * 4
})
fig_memory = px.bar(memory_df, x='Threshold Size', y='Memory Size (MB)', color='Type',
                     barmode='group', title="Memory Usage for Small Allocations")
st.plotly_chart(fig_memory)

st.write("""
### Performance Summary
The speedup factor shows the improvement in execution time when increasing the threshold size.
""")

# Performance Summary
improvements = {
    'Small Range': data['small_alloc']['exec_time'][0] / data['small_alloc']['exec_time'][-1],
    'Equal Size': data['equal_size']['exec_time'][0] / data['equal_size']['exec_time'][-1],
    'Large Range': data['large_range']['exec_time'][0] / data['large_range']['exec_time'][-1]
}
improve_df = pd.DataFrame(list(improvements.items()), columns=['Allocation Type', 'Speedup Factor'])
fig_improve = px.bar(improve_df, x='Allocation Type', y='Speedup Factor',
                      text_auto='.2f', title="Performance Improvement Summary")
st.plotly_chart(fig_improve)

st.write("""
## Conclusion
- The thread-safe version provides robustness for multi-threaded applications at the cost of performance.
- The non-thread-safe version is significantly faster but lacks concurrency control.
- Increasing the threshold size significantly reduces execution time and fragmentation, but beyond 1024B, improvements are marginal.

This report demonstrates that the best balance of speed and efficiency is achieved with the **1024-byte threshold** for general use cases.
""")
