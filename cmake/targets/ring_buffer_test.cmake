set(TEST_SOURCES
        test/tring_buffer.c
)

# Create the test executable
add_executable(ring_buffer_test ${TEST_SOURCES})
# Add Unity source files directly to the test target
target_sources(ring_buffer_test PRIVATE ${unity_SOURCE_DIR}/src/unity.c)
# Include the Unity headers
target_include_directories(ring_buffer_test PRIVATE ${unity_SOURCE_DIR}/src ${RBUFF_HEADERS})
# Link library to be tested
target_link_libraries(ring_buffer_test PRIVATE ${RBUFF_LIB})
# sanitizer
target_use_mem_sanitizer(ring_buffer_test ${RBUFF_TEST_CMEM_SANITIZER})

# Add the test to CTest
add_test(NAME RingBufferTest COMMAND ring_buffer_test)

# Optionally, run tests after build automatically
add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND}
        DEPENDS ring_buffer_test
        COMMENT "Running tests after build"
)