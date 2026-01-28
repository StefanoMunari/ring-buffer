Ring Buffer
===========

A lock-free ring buffer implementation in C with optional thread-safety support. This library provides efficient circular buffer operations with cycle tracking to distinguish between full and empty states.

Features
--------

* **Dual Architecture Support**: Compatible with both 32-bit and 64-bit systems
* **Flexible Memory Models**: Support for both scattered and linear memory layouts
* **Cycle/Wrap Tracking**: Uses MSB of index variables to track buffer wrap-around state
* **Thread Safety**: Optional mutex-based thread-safe operations via compile-time flag
* **Zero-Copy Design**: Efficient memory operations using direct pointer manipulation
* **Unit tested**: unit tests using C-unit for lock-free version, C++ googletest framework and pthread for thread-safe version

Overview
--------

The ring buffer uses indexing scheme where the MSB of the index values serves as a cycle flag.

* Write index (``cwrite_i``): Tracks write position [0, size-1] with MSB as wrap flag
* Read index (``cread_i``): Tracks read position [0, size-1] with MSB as wrap flag
* Buffer: Actual data storage area
* Buffer size: Total capacity in bytes
* Does not own allocated memory: caller responsible for allocation/deallocation of memory injected in ring buffer

Cycle Flag Mechanism
~~~~~~~~~~~~~~~~~~~~

The implementation uses the MSB of the index variables as a cycle flag:

* **32-bit systems**: Bit 31 is the cycle flag, bits 0-30 store the index
* **64-bit systems**: Bit 63 is the cycle flag, bits 0-62 store the index

This approach allows detecting the difference between:

* **Empty buffer**: ``read_index == write_index`` AND ``read_cycle == write_cycle``
* **Full buffer**: ``read_index == write_index`` AND ``read_cycle != write_cycle``

Wrap-Around Handling
~~~~~~~~~~~~~~~~~~~~

When a write or read operation reaches the end of the buffer, it automatically wraps around to the beginning. The operation is split into two ``memcpy`` calls:

1. Copy from current position to end of buffer
2. Copy remaining data from beginning of buffer

Compile and test
----------------

Following pre-processor macros can be used to compile a specific "flavour" of ring-buffer:

* Lock-free (non-thread-safe) version: *no preprocessor flags necessary*
* Test lock-free (non-thread-safe): *no preprocessor flags necessary*, run target "All C Tests"
* Thread-safe version: -DCMAKE_RING_BUFFER_THREAD_SAFE=1
* Test thread-safe: -DCMAKE_RING_BUFFER_THREAD_SAFE=1 -DRING_BUFFER_CPP_UNIT_TESTS=1, run target "gtest_main"

Usage
-----

Examples programs can be found in examples/ subdir