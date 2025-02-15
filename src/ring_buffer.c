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
struct RingBuffer make_ring_buffer_scattered(addr_t *cwrite_i, addr_t *cread_i,
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

struct RingBuffer make_ring_buffer_linear(addr_t *base_addr, uint32_t size)
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

void reset_ring_buffer(struct RingBuffer *ring_buffer)
{
	memset(ring_buffer, 0x00, sizeof(struct RingBuffer));
}

int32_t write(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size)
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
	if (size > ring_buffer->buffer_size)
		size = ring_buffer->buffer_size;// write at most buffer_size

	const addr_t unwrapped_wsize = wi + size;
	addr_t write_addr = (addr_t)ring_buffer->buffer + wi;

	addr_t cend_i = unwrapped_wsize % ring_buffer->buffer_size;
	if (cend_i > 0 && cend_i != unwrapped_wsize) {// wrapped/cycled
		if (wcycle != rcycle && cend_i > ri) {
			cend_i = ri;// capped by read index
		}
		const addr_t first_chunk =
			size - (ring_buffer->buffer_size - wi);
		memcpy((void *)write_addr, data, first_chunk);
		memcpy(ring_buffer->buffer, &data[first_chunk],
		       cend_i);
		// update cycle write index
		*(ring_buffer->cwrite_i) = cend_i % ring_buffer->buffer_size  | CYCLE_MASK;
		return first_chunk + cend_i;
	}
	if (wi != ri && wi + size > ri)
		size = ri;// capped by read index
	memcpy((void *)write_addr, data, size);
	// update cycle write index
	*(ring_buffer->cwrite_i) += ((wi + size) % ring_buffer->buffer_size & ~CYCLE_MASK);
	return size;
}

int32_t read(struct RingBuffer *ring_buffer, uint8_t *data, uint32_t size)
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
	if (size > ring_buffer->buffer_size)
		size = ring_buffer->buffer_size;// read at most buffer_size

	const addr_t unwrapped_rsize = ri + size;
	addr_t read_addr = (addr_t)ring_buffer->buffer + ri;

	addr_t cend_i = unwrapped_rsize % ring_buffer->buffer_size;
	if (cend_i > 0 && cend_i != unwrapped_rsize) {// wrapped/cycled
		if (rcycle != wcycle && cend_i > wi) {
			cend_i = wi;// capped by write index
		}
		const addr_t first_chunk =
			size - (ring_buffer->buffer_size - ri);
		memcpy(data, (void *)read_addr, first_chunk);
		memcpy(&data[first_chunk], ring_buffer->buffer, cend_i);
		// update cycle read index
		*(ring_buffer->cread_i) = cend_i % ring_buffer->buffer_size | CYCLE_MASK;
		return first_chunk + cend_i;
	}
	if (wi != ri && ri + size > wi)
		size = wi;// capped by write index
	memcpy(data, (void *)read_addr, size);
	// update cycle read index
	*(ring_buffer->cread_i) += (ri + size) % ring_buffer->buffer_size & ~CYCLE_MASK;
	return size;
}