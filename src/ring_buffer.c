#include "ring_buffer/ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
//#include <stdio.h>
// externs
const uint32_t WORD_SIZE = __SIZEOF_POINTER__;
const struct RingBuffer RING_BUFFER_INVALID = {
	.
	cwrite_i = NULL,
	.
	cread_i = NULL,
	.
	buffer = NULL,
	.
	buffer_size =
	0U,
#ifdef RING_BUFFER_THREAD_SAFE
	.
	mutex = NULL
#endif //RING_BUFFER_THREAD_SAFE
};
// consts
static const uint32_t LIN_BUFFER_OFFSET = WORD_SIZE * 2;
#if __SIZEOF_POINTER__ == 8
static const uint8_t INDEX_SIZE = 63U;
static const addr_t CYCLE_MASK = (1ULL << INDEX_SIZE); // set 64th bit
#endif
#if __SIZEOF_POINTER__ == 4
static const uint8_t INDEX_SIZE = 31U;
static const addr_t CYCLE_MASK = (1UL << INDEX_SIZE) // set 32nd bit
#endif

// private interface
static addr_t index__(addr_t addr)
{
	return addr & ~CYCLE_MASK;
}

static uint8_t cycle__(addr_t addr)
{
	return addr >> INDEX_SIZE;
}

// public interface
struct RingBuffer ring_buffer_make_scattered(addr_t *cwrite_i, addr_t *cread_i,
                                             addr_t *base_addr, uint32_t size
#ifdef RING_BUFFER_THREAD_SAFE
                                             , pthread_mutex_t *mutex
#endif //RING_BUFFER_THREAD_SAFE
                                             )
{
	if (cwrite_i == NULL || cread_i == NULL || base_addr == NULL || size <
		WORD_SIZE * 2
#ifdef RING_BUFFER_THREAD_SAFE
		|| mutex == NULL
#endif //RING_BUFFER_THREAD_SAFE
		)
		return RING_BUFFER_INVALID;
	*base_addr = 0U;
	*(base_addr + WORD_SIZE) = 0U;
	return (struct RingBuffer){
		.
		cwrite_i = cwrite_i,
		.
		cread_i = cread_i,
		.
		buffer = base_addr,
		.
		buffer_size = size,
#ifdef RING_BUFFER_THREAD_SAFE
		.
		mutex = mutex
#endif //RING_BUFFER_THREAD_SAFE
	};
}

struct RingBuffer ring_buffer_make_linear(addr_t *base_addr, uint32_t size
#ifdef RING_BUFFER_THREAD_SAFE
, pthread_mutex_t *mutex
#endif //RING_BUFFER_THREAD_SAFE
	)
{
	if (base_addr == NULL || size < (LIN_BUFFER_OFFSET + WORD_SIZE * 2)
#ifdef RING_BUFFER_THREAD_SAFE
	|| mutex == NULL
#endif //RING_BUFFER_THREAD_SAFE
		)
		return RING_BUFFER_INVALID;
	*base_addr = 0U;
	*(base_addr + WORD_SIZE) = 0U;
	return (struct RingBuffer){
		.
		cwrite_i = base_addr,
		.
		cread_i = base_addr + WORD_SIZE,
		.
		buffer = base_addr + LIN_BUFFER_OFFSET,
		.
		buffer_size = size - LIN_BUFFER_OFFSET,
#ifdef RING_BUFFER_THREAD_SAFE
		.
		mutex = mutex
#endif //RING_BUFFER_THREAD_SAFE
	};
}

void ring_buffer_reset(struct RingBuffer *ring_buffer)
{
#ifdef RING_BUFFER_THREAD_SAFE
	pthread_mutex_lock(ring_buffer->mutex);
#endif
	if (ring_buffer->cwrite_i)
		*ring_buffer->cwrite_i = 0x00;
	if (ring_buffer->cread_i)
		*ring_buffer->cread_i = 0x00;
	memset(ring_buffer->buffer, 0x00, ring_buffer->buffer_size);
	ring_buffer->buffer_size = 0x00;
	ring_buffer->cwrite_i = NULL;
	ring_buffer->cread_i = NULL;
	ring_buffer->buffer = NULL;
#ifdef RING_BUFFER_THREAD_SAFE
	pthread_mutex_unlock(ring_buffer->mutex);
#endif
}

typedef void (*Copy)(addr_t *x_addr, uint8_t *data, uint32_t size);
typedef void (*CopyWrapped)(addr_t *buffer, addr_t *x_addr, uint8_t *data, const addr_t first_chunk, addr_t cend_i);

static
int32_t transfer__(addr_t *buffer, const addr_t buffer_size, addr_t *cx_index, addr_t *cy_index, uint8_t *data,
	uint32_t size, Copy cp_cback, CopyWrapped cp_wrp_cback)
{
	// y info: the index which handles the other transfer op type
	const addr_t cy_addr = *(cy_index);
	const uint8_t ycycle = cycle__(cy_addr);
	const uint8_t yi = index__(cy_addr);
	// x info: the index which handles the requested transfer type
	const addr_t cx_addr = *(cx_index);
	uint8_t xcycle = cycle__(cx_addr);
	const addr_t xi = index__(cx_addr);

	if (size > buffer_size)
		size = buffer_size;// transfer at most buffer_size

	const addr_t unwrapped_xsize = xi + size;
	addr_t x_addr = (addr_t)buffer + xi;

	addr_t cend_i = unwrapped_xsize % buffer_size;
	if (cend_i > 0 && cend_i != unwrapped_xsize) {// wrapped/cycled
		if (xcycle == ycycle && cend_i > yi) {
			cend_i = yi;// capped by y index
		}
		const addr_t first_chunk = buffer_size - xi;
		// DEBUG STRING
		// printf("\ns:%u,x:%p,b:%p,f:%u,xi:%u,yi:%u\n", buffer_size, (void *)x_addr, (void *)buffer, first_chunk, xi, yi);
		cp_wrp_cback(buffer, (addr_t *)x_addr, data, first_chunk, cend_i);
		// update cycle x index
		*(cx_index) = cend_i % buffer_size | CYCLE_MASK;
		return first_chunk + cend_i;
	}
	if (xi < yi && xi + size > yi)
		size = yi;// capped by y index
	cp_cback((addr_t *)x_addr, data, size);
	// update cycle x index
	*(cx_index) = ((xi + size) % buffer_size & ~CYCLE_MASK);
	return size;
}

static
void copy_write__(addr_t *x_addr, uint8_t *data, uint32_t size) {
	memcpy(x_addr, data, size);
}

static
void copy_write_wrapped__(addr_t *buffer, addr_t *x_addr, uint8_t *data, const addr_t first_chunk, addr_t cend_i) {
	memcpy(x_addr, data, first_chunk);
	memcpy(buffer, &data[first_chunk], cend_i);
}

int32_t ring_buffer_write(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size)
{
#ifdef RING_BUFFER_THREAD_SAFE
	pthread_mutex_lock(ring_buffer->mutex);
#endif
	// read info
	const addr_t cr_addr = *(ring_buffer->cread_i);
	const uint8_t rcycle = cycle__(cr_addr);
	const uint8_t ri = index__(cr_addr);
	// write info
	const addr_t cw_addr = *(ring_buffer->cwrite_i);
	uint8_t wcycle = cycle__(cw_addr);
	const addr_t wi = index__(cw_addr);

	if ((wcycle == rcycle && wi < ri) || (wcycle != rcycle && wi > ri)) {
#ifdef RING_BUFFER_THREAD_SAFE
		pthread_mutex_unlock(ring_buffer->mutex);
#endif
		return -1; // invalid buffer
	}
	if (wcycle != rcycle && wi == ri) {
#ifdef RING_BUFFER_THREAD_SAFE
		pthread_mutex_unlock(ring_buffer->mutex);
#endif
		return 0; // buffer is full
	}
#ifdef RING_BUFFER_THREAD_SAFE
	const int32_t written = transfer__(ring_buffer->buffer, ring_buffer->buffer_size, ring_buffer->cwrite_i, ring_buffer->cread_i, data,
		size, copy_write__, copy_write_wrapped__);
	pthread_mutex_unlock(ring_buffer->mutex);
	return written;
#else
	// x = write, y = read
	return transfer__(ring_buffer->buffer, ring_buffer->buffer_size, ring_buffer->cwrite_i, ring_buffer->cread_i, data,
		size, copy_write__, copy_write_wrapped__);
#endif
}

static
void copy_read__(addr_t *x_addr, uint8_t *data, uint32_t size) {
	memcpy(data, x_addr, size);
}

static
void copy_read_wrapped__(addr_t *buffer, addr_t *x_addr, uint8_t *data, const addr_t first_chunk, addr_t cend_i) {
	memcpy(data, x_addr, first_chunk);
	memcpy(&data[first_chunk], buffer, cend_i);
}

int32_t ring_buffer_read(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size)
{
#ifdef RING_BUFFER_THREAD_SAFE
	pthread_mutex_lock(ring_buffer->mutex);
#endif
	// read info
	const addr_t cr_addr = *(ring_buffer->cread_i);
	const uint8_t rcycle = cycle__(cr_addr);
	const uint8_t ri = index__(cr_addr);
	// write info
	const addr_t cw_addr = *(ring_buffer->cwrite_i);
	uint8_t wcycle = cycle__(cw_addr);
	const addr_t wi = index__(cw_addr);

	if ((rcycle != wcycle && ri < wi) || (rcycle == wcycle && wi < ri)) {
#ifdef RING_BUFFER_THREAD_SAFE
		pthread_mutex_unlock(ring_buffer->mutex);
#endif
		return -1; // invalid buffer
	}
	if (ring_buffer->cread_i == ring_buffer->cwrite_i) {
#ifdef RING_BUFFER_THREAD_SAFE
		pthread_mutex_unlock(ring_buffer->mutex);
#endif
		return 0; // buffer is empty
	}
#ifdef RING_BUFFER_THREAD_SAFE
	const int32_t read = transfer__(ring_buffer->buffer, ring_buffer->buffer_size, ring_buffer->cread_i, ring_buffer->cwrite_i, data,
		size, copy_read__, copy_read_wrapped__);
	pthread_mutex_unlock(ring_buffer->mutex);
	return read;
#else
	// x = read, y = write
	return transfer__(ring_buffer->buffer, ring_buffer->buffer_size, ring_buffer->cread_i, ring_buffer->cwrite_i, data,
		size, copy_read__, copy_read_wrapped__);
#endif
}
