cmake_minimum_required(VERSION 3.30)
project(ring-buffer C)
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
# Tools
#=============================
include(cmake/tools/clang_sanitizer.cmake)
include(cmake/tools/clang_tidy.cmake)
include(cmake/tools/test.cmake)
#=============================
# Targets
#=============================
set(RBUFF_LIB ring_buff)
set(RBUFF_HEADERS include/)
file(GLOB_RECURSE SOURCES "src/*.c")
add_library(${RBUFF_LIB} ${SOURCES})
target_include_directories(${RBUFF_LIB} PUBLIC ${RBUFF_HEADERS})

include(cmake/targets/ring_buffer_test.cmake)
