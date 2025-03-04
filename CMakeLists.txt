cmake_minimum_required(VERSION 3.30)
project(ring-buffer C CXX)
set(RBUFF_PROJECT_NAME "${CMAKE_PROJECT_NAME}")
#=============================
# Requirements
#=============================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# default overridable variables (pass -D<NAME_VAR>=VALUE in CMAKE profile)
set(RBUFF_CLANG_TIDY "ON" CACHE BOOL "")
set(RBUFF_CMEM_SANITIZER "ON" CACHE BOOL "")
#=============================
# Version
#=============================
include(cmake/tools/version.cmake)
include(cmake/tools/git.cmake)
GetGitDescription(RINGBUFFER_GIT_SHA1 RINGBUFFER_GIT_DATE RINGBUFFER_GIT_COMMIT_SUBJECT)
configure_file(ring_buffer_version.in ${CMAKE_CURRENT_BINARY_DIR}/ring_buffer_version.c @ONLY)
#=============================
# Tools
#=============================
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools/clang_sanitizer.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools/clang_tidy.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools/test.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools/googletest.cmake)
#=============================
# Targets
#=============================
set(RBUFF_LIB ring_buff)
set(RBUFF_HEADERS include/)
file(GLOB_RECURSE SOURCES "src/*.c")
add_library(${RBUFF_LIB} ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/ring_buffer_version.c)
target_include_directories(${RBUFF_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${RBUFF_HEADERS})

if (${CMAKE_RING_BUFFER_THREAD_SAFE})
    target_compile_definitions(${RBUFF_LIB} PRIVATE RING_BUFFER_THREAD_SAFE=${CMAKE_RING_BUFFER_THREAD_SAFE})
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/ring_buffer_example.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/ring_buffer_test.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/targets/ring_buffer_test_multithread.cmake)