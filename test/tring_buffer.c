#include <string.h>
#include <stdlib.h>

#include "unity.h"
#include "ring_buffer/ring_buffer.h"

static const int mem_size = 64;
static addr_t *mem = NULL;
static struct RingBuffer rb;

void setUp(void) {
  // Set up code for each test
  mem = (addr_t *)calloc(WORD_SIZE*2 + mem_size, 1);
  rb = make_ring_buffer_linear(mem, mem_size);
}

void tearDown(void) {
  // Clean up after each test
  reset_ring_buffer(&rb);
  free(mem);
}

void trbuf_ctor_linear(void) {
  const int expected_buffer_size = mem_size-WORD_SIZE*2;
  struct RingBuffer rbl = make_ring_buffer_linear(mem, mem_size);
  TEST_ASSERT_EQUAL(rbl.buffer_size, expected_buffer_size);
}

void trbuf_ctor_linear_fail(void) {
  struct RingBuffer rbl = make_ring_buffer_linear(mem, 0U);
  TEST_ASSERT_EQUAL_MEMORY(&rbl, &RING_BUFFER_INVALID, sizeof(rbl));
}

void trbuf_ctor_scattered(void) {
  const int expected_buffer_size = mem_size-WORD_SIZE*2;
  struct RingBuffer rbs = make_ring_buffer_scattered(mem, &mem[WORD_SIZE], &mem[WORD_SIZE*2], expected_buffer_size);
  TEST_ASSERT_EQUAL(rbs.buffer_size, expected_buffer_size);
}

void trbuf_ctor_scattered_fail(void) {
  const int expected_buffer_size = mem_size-WORD_SIZE*2;
  struct RingBuffer rbs = make_ring_buffer_scattered(mem, &mem[WORD_SIZE], NULL, expected_buffer_size);
  TEST_ASSERT_EQUAL_MEMORY(&rbs, &RING_BUFFER_INVALID, sizeof(rbs));
}

static void _rbuf_write_read_bigbuffer(const int msg_size) {
  char expected_msg[rb.buffer_size];
  memset(expected_msg, 0x42, rb.buffer_size);
  // WRITE
  char msg[msg_size];
  memset(msg, 0x42, msg_size);
  int32_t result = write(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, rb.buffer_size);
  // READ
  memset(msg, 0x00, msg_size);
  result = read(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, rb.buffer_size);
  TEST_ASSERT_EQUAL_MEMORY(expected_msg, msg, rb.buffer_size);
}

void trbuf_write_read_bigbuffer0(void) {
  _rbuf_write_read_bigbuffer(mem_size*3);
}

void trbuf_write_read_bigbuffer1(void) {
  _rbuf_write_read_bigbuffer(mem_size*3 - 3);
}

void trbuf_write_read_notwrapped(void) {
  char expected_msg[] = "Hello World!";
  // WRITE
  char msg[] = "Hello World!";
  const int msg_size = strlen(msg);
  int32_t result = write(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, msg_size);
  // READ
  memset(msg, 0, msg_size);
  result = read(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, msg_size);
  TEST_ASSERT_EQUAL_MEMORY(expected_msg, msg, msg_size);
}

void trbuf_write_read_notwrapped_fail(void) {
  // WRITE
  char msg[] = "Hello World!";
  const int msg_size = strlen(msg);
  // BREAK RBUF
  *(rb.cread_i) = *(rb.cwrite_i) + 1;
  int32_t result = write(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, -1);
  // BREAK RBUF
  *(rb.cread_i) = *(rb.cwrite_i) + WORD_SIZE;
  // READ
  memset(msg, 0, msg_size);
  result = read(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, -1);
}

void trbuf_write_read_wrapped(void) {
  int wrapped_size =  rb.buffer_size + rb.buffer_size/2;
  int32_t expected_size0 = rb.buffer_size;
  int32_t expected_size1 = wrapped_size - expected_size0;
  uint8_t write_buf[wrapped_size];
  memset(write_buf, 0x42, wrapped_size);
  uint8_t read_buf[wrapped_size];
  memset(read_buf, 0x00, wrapped_size);
  // WRITE
  int32_t result = write(&rb, write_buf, wrapped_size);
  TEST_ASSERT_EQUAL(result, expected_size0);
  // READ
  result = read(&rb, read_buf, wrapped_size);
  TEST_ASSERT_EQUAL(result, expected_size0);
  TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, expected_size0);
  // WRITE
  result = write(&rb, &write_buf[expected_size0], wrapped_size-expected_size0);
  TEST_ASSERT_EQUAL(result, expected_size1);
  // READ
  result = read(&rb, &read_buf[expected_size0], wrapped_size-expected_size0);
  TEST_ASSERT_EQUAL(result, expected_size1);
  TEST_ASSERT_EQUAL_MEMORY(write_buf, read_buf, wrapped_size);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(trbuf_ctor_linear);
  RUN_TEST(trbuf_ctor_linear_fail);
  RUN_TEST(trbuf_ctor_scattered);
  RUN_TEST(trbuf_ctor_scattered_fail);
  RUN_TEST(trbuf_write_read_bigbuffer0);
  RUN_TEST(trbuf_write_read_bigbuffer1);
  RUN_TEST(trbuf_write_read_notwrapped);
  RUN_TEST(trbuf_write_read_notwrapped_fail);
  RUN_TEST(trbuf_write_read_wrapped);
  return UNITY_END();
}