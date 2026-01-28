add_executable(ring_buffer_example examples/example.c)
target_include_directories(ring_buffer_example PRIVATE ${RBUFF_HEADERS})
target_link_libraries(ring_buffer_example PRIVATE ${RBUFF_LIB})
target_use_mem_sanitizer(ring_buffer_example ${RBUFF_EXAMPLE_CMEM_SANITIZER})