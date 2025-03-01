#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <random>
extern "C" {
#include "ring_buffer/ring_buffer.h"
}
#ifdef RING_BUFFER_THREAD_SAFE
static constexpr auto MEM_SIZE = 4096;
static addr_t MEM[MEM_SIZE];
static RingBuffer RING_BUFFER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int get_random(int min, int max) {
    // Random device and generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);

    return dist(gen);
}

static void Producer() {
    const auto data_size = 256U;
    uint8_t data[data_size];
    memset(data, 0x42, data_size);

    ring_buffer_write(&RING_BUFFER, data, data_size);
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(get_random(1, 900)));
        int written = ring_buffer_write(&RING_BUFFER, data, data_size);
        if (written)
            printf("p");
    }
}

static void Consumer() {
    const auto data_size = 256U;
    uint8_t data[data_size];
    memset(data, 0x00, data_size);
    uint8_t expected_data[data_size];
    memset(expected_data, 0x42, data_size);

    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(get_random(1, 900)));
        memset(data, 0x00, data_size);
        int ret = ring_buffer_read(&RING_BUFFER, data, data_size);
        if (ret == data_size) {
            printf("c");
            EXPECT_EQ(memcmp(data, expected_data, data_size), 0);
        }else if (ret > 0) {// something when wrong
            EXPECT_EQ(1,0);
        }
    }
}

TEST(RingBufferTest, Producer1Consumer1Multithread) {
    RING_BUFFER = ring_buffer_make_linear(MEM, MEM_SIZE, &mutex);
    std::thread producer(Producer);
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::thread consumer(Consumer);
    producer.join();
    consumer.join();
}

TEST(RingBufferTest, Producer1Consumer2Multithread) {
    RING_BUFFER = ring_buffer_make_linear(MEM, MEM_SIZE, &mutex);
    std::thread producer(Producer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread consumer(Consumer);
    std::thread consumer1(Consumer);
    producer.join();
    consumer.join();
    consumer1.join();
}

TEST(RingBufferTest, Producer2Consumer2Multithread) {
    RING_BUFFER = ring_buffer_make_linear(MEM, MEM_SIZE, &mutex);
    std::thread producer(Producer);
    std::thread producer1(Producer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread consumer(Consumer);
    std::thread consumer1(Consumer);
    producer.join();
    producer1.join();
    consumer.join();
    consumer1.join();
}

#endif //RING_BUFFER_THREAD_SAFE
// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

