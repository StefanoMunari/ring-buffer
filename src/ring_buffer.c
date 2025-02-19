// SPDX-License-Identifier: GPL-2.0
#include "ring_buffer/ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
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
	0U
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
static addr_t _index(addr_t addr)
{
	return addr & ~CYCLE_MASK;
}

static uint8_t _cycle(addr_t addr)
{
	return addr >> INDEX_SIZE;
}

// public interface
struct RingBuffer ring_buffer_make_scattered(addr_t *cwrite_i, addr_t *cread_i,
                                             addr_t *base_addr, uint32_t size)
{
	if (cwrite_i == NULL || cread_i == NULL || base_addr == NULL || size <
		WORD_SIZE * 2)
		return RING_BUFFER_INVALID;
	return (struct RingBuffer){
		.
		cwrite_i = cwrite_i,
		.
		cread_i = cread_i,
		.
		buffer = base_addr,
		.
		buffer_size = size,
	};
}

struct RingBuffer ring_buffer_make_linear(addr_t *base_addr, uint32_t size)
{
	if (base_addr == NULL || size < (LIN_BUFFER_OFFSET + WORD_SIZE * 2))
		return RING_BUFFER_INVALID;
	return (struct RingBuffer){
		.
		cwrite_i = base_addr,
		.
		cread_i = base_addr + WORD_SIZE,
		.
		buffer = base_addr + LIN_BUFFER_OFFSET,
		.
		buffer_size = size - LIN_BUFFER_OFFSET,
	};
}

void ring_buffer_reset(struct RingBuffer *ring_buffer)
{
	memset(ring_buffer, 0x00, sizeof(struct RingBuffer));
}

typedef void (*Copy)(void *x_addr, uint8_t *data, uint32_t size);
typedef void (*CopyWrapped)(addr_t *buffer, void *x_addr, uint8_t *data, const addr_t first_chunk, addr_t cend_i);

int32_t _transfer(addr_t *buffer, const addr_t buffer_size, addr_t *cx_index, addr_t *cy_index, uint8_t *data,
	uint32_t size, Copy cp_cback, CopyWrapped cp_wrp_cback)
{
	// y info: the index which handles the other transfer op type
	const addr_t cy_addr = *(cy_index);
	const uint8_t ycycle = _cycle(cy_addr);
	const uint8_t yi = _index(cy_addr);
	// x info: the index which handles the requested transfer type
	const addr_t cx_addr = *(cx_index);
	uint8_t xcycle = _cycle(cx_addr);
	const addr_t xi = _index(cx_addr);

	if (size > buffer_size)
		size = buffer_size;// transfer at most buffer_size

	const addr_t unwrapped_xsize = xi + size;
	addr_t x_addr = (addr_t)buffer + xi;

	addr_t cend_i = unwrapped_xsize % buffer_size;
	if (cend_i > 0 && cend_i != unwrapped_xsize) {// wrapped/cycled
		if (xcycle != ycycle && cend_i > yi) {
			cend_i = yi;// capped by y index
		}
		const addr_t first_chunk =
			size - (buffer_size - xi);
		cp_wrp_cback(buffer, (void *)x_addr, data, first_chunk, cend_i);
		// update cycle x index
		*(cx_index) = cend_i % buffer_size | CYCLE_MASK;
		return first_chunk + cend_i;
	}
	if (xi != yi && xi + size > yi)
		size = yi;// capped by y index
	cp_cback((void *)x_addr, data, size);
	// update cycle x index
	*(cx_index) += ((xi + size) % buffer_size & ~CYCLE_MASK);
	return size;
}

void _copy_write(void *x_addr, uint8_t *data, uint32_t size) {
	memcpy(x_addr, data, size);
}

void _copy_write_wrapped(addr_t *buffer, void *x_addr, uint8_t *data, const addr_t first_chunk, addr_t cend_i) {
	memcpy(x_addr, data, first_chunk);
	memcpy(buffer, &data[first_chunk], cend_i);
}

int32_t ring_buffer_write(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size)
{
	// read info
	const addr_t cr_addr = *(ring_buffer->cread_i);
	const uint8_t rcycle = _cycle(cr_addr);
	const uint8_t ri = _index(cr_addr);
	// write info
	const addr_t cw_addr = *(ring_buffer->cwrite_i);
	uint8_t wcycle = _cycle(cw_addr);
	const addr_t wi = _index(cw_addr);

	if ((wcycle == rcycle && wi < ri) || (wcycle != rcycle && wi > ri))
		return -1; // invalid buffer
	if (wcycle != rcycle && wi == ri)
		return 0; // buffer is full
	// x = write, y = read
	return _transfer(ring_buffer->buffer, ring_buffer->buffer_size, ring_buffer->cwrite_i, ring_buffer->cread_i, data,
		size, _copy_write, _copy_write_wrapped);
}

void _copy_read(void *x_addr, uint8_t *data, uint32_t size) {
	memcpy(data, x_addr, size);
}

void _copy_read_wrapped(addr_t *buffer, void *x_addr, uint8_t *data, const addr_t first_chunk, addr_t cend_i) {
	memcpy(data, x_addr, first_chunk);
	memcpy(&data[first_chunk], buffer, cend_i);
}

int32_t ring_buffer_read(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size)
{
	// read info
	const addr_t cr_addr = *(ring_buffer->cread_i);
	const uint8_t rcycle = _cycle(cr_addr);
	const uint8_t ri = _index(cr_addr);
	// write info
	const addr_t cw_addr = *(ring_buffer->cwrite_i);
	uint8_t wcycle = _cycle(cw_addr);
	const addr_t wi = _index(cw_addr);

	if ((rcycle != wcycle && ri < wi) || (rcycle == wcycle && wi < ri))
		return -1; // invalid buffer
	if (ring_buffer->cread_i == ring_buffer->cwrite_i)
		return 0; // buffer is empty
	// x = read, y = write
	return _transfer(ring_buffer->buffer, ring_buffer->buffer_size, ring_buffer->cread_i, ring_buffer->cwrite_i, data,
		size, _copy_read, _copy_read_wrapped);
}