#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ring_buffer/ring_buffer.h>
#include <ring_buffer/ring_buffer_version.h>

int main() {
  printf("ring_buffer_version:%s\n", ring_buffer_get_version());
  printf("ring_buffer_git:%s\n", ring_buffer_get_git_sha());

  struct RingBuffer rb;
  const int mem_size = 64;
  addr_t *mem = NULL;

  mem = (addr_t *)calloc(WORD_SIZE*2 + mem_size, 1);
  rb = ring_buffer_make_linear(mem, mem_size);

  // WRITE
  char msg[] = "Hello World!";
  const int msg_size = strlen(msg);
  int32_t result = ring_buffer_write(&rb, msg, msg_size);
  printf("Written(%i) bytes\n", result);

  // READ
  memset(msg, 0, msg_size);
  result = ring_buffer_read(&rb, msg, msg_size);
  printf("Read(%i) bytes\n", result);
  printf("Read:%s\n", msg);
}
