#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <random>

extern "C" {
#include "ring_buffer/ring_buffer.h"
}
#ifdef RING_BUFFER_THREAD_SAFE

static int get_random(int min, int max)
{
		// Random device and generator
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dist(min, max);

		return dist(gen);
}

static void Producer(struct RingBuffer *ring_buffer)
{
		const auto data_size = 256U;
		uint8_t data[data_size];
		memset(data, 0x42, data_size);
		int fail = 0, success = 0;

		for (int i = 0; i < 10; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(get_random(50 + success, 500 + success * i)));
				int written = ring_buffer_write(ring_buffer, data, data_size);
				if (written > 0) {
						++success;
				} else {
						++fail;
				}
		}
		printf("producer:stats:fail:%d,success:%d\n", fail, success);
}

static void Consumer(struct RingBuffer *ring_buffer)
{
		const auto data_size = 256U;
		uint8_t data[data_size];
		memset(data, 0x00, data_size);
		uint8_t expected_data[data_size];
		memset(expected_data, 0x42, data_size);
		int fail = 0, success = 0;
		for (int i = 0; i < 10; i++) {
				std::this_thread::sleep_for(std::chrono::milliseconds(get_random(50, 500)));
				memset(data, 0x00, data_size);
				int ret = ring_buffer_read(ring_buffer, data, data_size);
				if (ret == data_size || ret == (ring_buffer->buffer_size % data_size)) {
						++success;
						EXPECT_EQ(memcmp(data, expected_data, ret), 0);
				} else if (ret > 0) {
						// something went wrong: corrupted state
						printf("rbs:%lu,ds:%d\n", ring_buffer->buffer_size, data_size);
						EXPECT_EQ(ret, 0);
				} else {
						++fail;
				}
		}
		printf("consumer:stats:fail:%d,success:%d\n", fail, success);
}

TEST(RingBufferTest, Producer1Consumer1Multithread)
{
		const auto mem_size = 528;
		addr_t *mem = (addr_t *)calloc(mem_size, 1);
		RingBuffer ring_buffer;
		ring_buffer = ring_buffer_make_linear(mem, mem_size - WORD_SIZE * 2);
		std::thread producer(Producer, &ring_buffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		std::thread consumer(Consumer, &ring_buffer);
		producer.join();
		consumer.join();
		free(mem);
}

TEST(RingBufferTest, Producer1Consumer2Multithread)
{
		const auto mem_size = 528;
		addr_t *mem = (addr_t *)calloc(mem_size, 1);
		RingBuffer ring_buffer;
		ring_buffer = ring_buffer_make_linear(mem, mem_size - WORD_SIZE * 2);
		EXPECT_NE(memcmp(&ring_buffer, &RING_BUFFER_INVALID, sizeof(RING_BUFFER_INVALID)), 0);
		std::thread producer(Producer, &ring_buffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		std::thread consumer(Consumer, &ring_buffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::thread consumer1(Consumer, &ring_buffer);
		producer.join();
		consumer.join();
		consumer1.join();
		free(mem);
}

TEST(RingBufferTest, Producer1Consumer3Multithread)
{
		const auto mem_size = 528;
		addr_t *mem = (addr_t *)calloc(mem_size, 1);
		RingBuffer ring_buffer;
		ring_buffer = ring_buffer_make_linear(mem, mem_size - WORD_SIZE * 2);
		EXPECT_NE(memcmp(&ring_buffer, &RING_BUFFER_INVALID, sizeof(RING_BUFFER_INVALID)), 0);
		std::thread producer(Producer, &ring_buffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		std::thread consumer(Consumer, &ring_buffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::thread consumer1(Consumer, &ring_buffer);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::thread consumer2(Consumer, &ring_buffer);
		producer.join();
		consumer.join();
		consumer1.join();
		consumer2.join();
		free(mem);
}

#endif //RING_BUFFER_THREAD_SAFE
// Main function for running tests
int main(int argc, char **argv)
{
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
}
