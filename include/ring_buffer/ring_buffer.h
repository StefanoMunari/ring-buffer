
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

// Architecture addressing:
// if none supported -> dont compile
#if __SIZEOF_POINTER__ == 8
typedef uint64_t addr_t;
#endif
#if __SIZEOF_POINTER__ == 4
typedef uint32_t addr_t;
#endif

struct RingBuffer {
	// ptr to cycle write index
	addr_t *cwrite_i;
	// ptr to cycle read index
	addr_t *cread_i;
	// ptr to data
	addr_t *buffer;
	// size of buffer in bytes
	addr_t buffer_size;
} __attribute__((packed, aligned(sizeof(addr_t))));

extern const uint32_t WORD_SIZE;
extern const struct RingBuffer RING_BUFFER_INVALID;

struct RingBuffer ring_buffer_make_scattered(addr_t *cwrite_i, addr_t *cread_i,
                                             addr_t *base_addr, uint32_t size);

/**
 * creates a ring buffer from a chunk of allocated contiguous memory
 * @param base_addr base address of memory chunk
 * @param size size of memory chunk
 * @return a ring buffer instance, RING_BUFFER_INVALID (buffer_size = 0) if fail
 */
struct RingBuffer ring_buffer_make_linear(addr_t *base_addr, uint32_t size);

/**
 * reset a ring buffer w/out deallocating mem which is not owned by ring buffer
 * @param ring_buffer the buffer to reset
 */
void ring_buffer_reset(struct RingBuffer *ring_buffer);

/**
 * write size data into ring_buffer from cwrite_i position
 * @param ring_buffer object to write data into
 * @param data buffer from which data is read
 * @param size number of bytes to be written
 * @return number of bytes written, -1 otherwise
 */
int32_t ring_buffer_write(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size);

// read size data from ring_buffer cread_i position
// return number of bytes read
int32_t ring_buffer_read(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size);


#endif //RING_BUFFER_H
