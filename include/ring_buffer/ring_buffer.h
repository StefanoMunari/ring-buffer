
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#ifdef RING_BUFFER_THREAD_SAFE
#include <pthread.h>
#endif //RING_BUFFER_THREAD_SAFE

// Architecture addressing:
// if none supported -> dont compile
#if __SIZEOF_POINTER__ == 8
typedef uint64_t addr_t;
#endif
#if __SIZEOF_POINTER__ == 4
typedef uint32_t addr_t;
#endif

/**
 * Ring buffer structure:
 * cwrite_i: ptr to write index (0, size -1) which MSb is used as cycle flag (0=write !wrapped, 1=write wrapped)
 * cread_i: ptr to read index (0, size -1) which MSb is used as cycle flag (0=read !wrapped, 1=read wrapped)
 * buffer: ptr to actual buffer used to store data in ring buffer
 * buffer_size: size of buffer in bytes
 */
struct RingBuffer {
	// ptr to cycle write index
	addr_t *cwrite_i;
	// ptr to cycle read index
	addr_t *cread_i;
	// ptr to data
	addr_t *buffer;
	// size of buffer in bytes
	addr_t buffer_size;
#ifdef RING_BUFFER_THREAD_SAFE
	pthread_mutex_t *mutex;
#endif //RING_BUFFER_THREAD_SAFE
} __attribute__((packed, aligned(sizeof(addr_t))));

extern const uint32_t WORD_SIZE;
/**
 * instantiation of an invalid buffer (not usable) w a buffer_size = 0 and nullptrs.
 */
extern const struct RingBuffer RING_BUFFER_INVALID;

/**
 * creates a ring buffer from scattered memory.
 * Indexes and buffer may be range of not-contiguous addresses. Example:
 * cwrite_i (write index) = 0x0A000000
 * cread_i (read index) = 0x0C008000
 * base_addr (buffer ptr) = 0x0B000000
 * size (of buffer) = 0x200
 * @param cwrite_i: write index which contains also cycle (flag to signal if last write wrapped in ring buffer)
 * @param cread_i: read index which contains also cycle (flag to signal if last read wrapped in ring buffer)
 * @param base_addr: ptr to start of buffer allocated memory
 * @param size: size of buffer to use
 * @return a ring buffer instance initialized with input parameters, RING_BUFFER_INVALID (buffer_size = 0) if fail
 * NOTE: indexes (cwrite_i, cread_i) values ranges { 0, size-1 }
 */
struct RingBuffer ring_buffer_make_scattered(addr_t *cwrite_i, addr_t *cread_i,
                                             addr_t *base_addr, uint32_t size
#ifdef RING_BUFFER_THREAD_SAFE
                                             ,pthread_mutex_t *mutex
#endif //RING_BUFFER_THREAD_SAFE
                                             );

/**
 * creates a ring buffer from a chunk of allocated contiguous memory.
 * Part of contiguous mem is used to handle indexes (cwrite_i, cread_i)
 * remaining is used for buffer itself
 * @param base_addr base address of memory chunk
 * @param size size of memory chunk
 * @return a ring buffer instance, RING_BUFFER_INVALID (buffer_size = 0) if fail
 */
struct RingBuffer ring_buffer_make_linear(addr_t *base_addr, uint32_t size
#ifdef RING_BUFFER_THREAD_SAFE
											, pthread_mutex_t *mutex
#endif //RING_BUFFER_THREAD_SAFE
											);

/**
 * reset a ring buffer w/out deallocating mem which is not owned by ring buffer
 * @param ring_buffer the buffer to reset
 */
void ring_buffer_reset(struct RingBuffer *ring_buffer);

/**
 * write size bytes from data into ring_buffer.buffer from cwrite_i position
 * @param ring_buffer object to write data into
 * @param data buffer from which data is read
 * @param size number of bytes to be written
 * @return number of bytes written, -1 otherwise
 */
int32_t ring_buffer_write(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size);

/**
 * read size bytes from ring_buffer.buffer into data from cread_i position
 * @param ring_buffer object to read data from
 * @param data buffer to which data is written
 * @param size number of bytes to be read
 * @return number of bytes read, -1 otherwise
 */
int32_t ring_buffer_read(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size);


#endif //RING_BUFFER_H
