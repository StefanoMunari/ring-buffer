#include <string.h>
#include <stdlib.h>

#include "unity.h"
#include "ring_buffer/ring_buffer.h"

static const int mem_size = 1024;
static addr_t *mem = NULL;
static struct RingBuffer rb;

void setUp(void) {
  // Set up code for each test
  mem = (addr_t *)malloc(mem_size * sizeof(addr_t));
  rb = make_ring_buffer_linear(mem, mem_size);
}

void tearDown(void) {
  // Clean up after each test
  free(mem);
}

void trbuf_ctor_linear(void) {
  const int expected_buffer_size = mem_size-WORD_SIZE*2;
  struct RingBuffer rbl = make_ring_buffer_linear(mem, mem_size);
  TEST_ASSERT_EQUAL(rbl.buffer_size, expected_buffer_size);

}

void trbuf_ctor_scattered(void) {
  const int expected_buffer_size = mem_size-WORD_SIZE*2;
  struct RingBuffer rbs = make_ring_buffer_scattered(mem, &mem[WORD_SIZE], &mem[WORD_SIZE*2], expected_buffer_size);
  TEST_ASSERT_EQUAL(rbs.buffer_size, expected_buffer_size);
}

void trbuf_write_read_notwrapped(void) {
  char msg[] = "Hello World!";
  const int msg_size = strlen(msg);
  int32_t result = write(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, msg_size);

  char expected_msg[] = "Hello World!";
  memset(msg, 0, msg_size);
  result = read(&rb, msg, msg_size);
  TEST_ASSERT_EQUAL(result, msg_size);
  TEST_ASSERT_EQUAL_MEMORY(expected_msg, msg, msg_size);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(trbuf_ctor_linear);
  RUN_TEST(trbuf_ctor_scattered);
  RUN_TEST(trbuf_write_read_notwrapped);
  return UNITY_END();
}