#=============================
# Test
#=============================
if(RING_BUFFER_CPP_UNIT_TESTS)
    add_executable(ring_buffer_test_multithread
            test_mt/tring_buffer_multithread.cpp)

    target_include_directories(ring_buffer_test_multithread PRIVATE ${RBUFF_HEADERS})
    target_link_libraries(ring_buffer_test_multithread PRIVATE gmock_main ${RBUFF_LIB})
    target_compile_definitions(ring_buffer_test_multithread PRIVATE RING_BUFFER_THREAD_SAFE=${CMAKE_RING_BUFFER_THREAD_SAFE})
    target_compile_options(ring_buffer_test_multithread PRIVATE -fsanitize=thread)
    target_link_options(ring_buffer_test_multithread PRIVATE -fsanitize=thread)
    # sanitizer
    target_use_mem_sanitizer(ring_buffer_test_multithread ${RBUFF_TESTMT_CMEM_SANITIZER})

    add_test(NAME test_ring_buffer_cpp COMMAND ring_buffer_test_multithread)
    # Enable XML report
    include(GoogleTest)
    gtest_discover_tests(ring_buffer_test_multithread XML_OUTPUT_DIR "${CMAKE_BINARY_DIR}/test_output/")
    # Enable automatic test discovery
    enable_testing()
endif()