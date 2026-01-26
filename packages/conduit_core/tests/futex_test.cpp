#include "conduit_core/internal/futex.hpp"
#include "conduit_core/internal/ring_buffer.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

using namespace conduit::internal;
using namespace std::chrono_literals;

class FutexTest : public ::testing::Test {
protected:
    std::unique_ptr<uint8_t[]> allocate_region(const RingBufferConfig& config) {
        size_t size = calculate_region_size(config);
        auto region = std::make_unique<uint8_t[]>(size);
        std::memset(region.get(), 0, size);
        return region;
    }
};

TEST_F(FutexTest, test_futex_wake_wait) {
    std::atomic<uint32_t> futex_word{0};
    std::atomic<bool> thread_started{false};
    std::atomic<bool> thread_woken{false};

    // Start waiter thread
    std::thread waiter([&]() {
        thread_started.store(true, std::memory_order_release);
        futex_wait(&futex_word, 0);  // wait while value is 0
        thread_woken.store(true, std::memory_order_release);
    });

    // Wait for thread to start
    while (!thread_started.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(1ms);
    }

    std::this_thread::sleep_for(10ms);  // ensure thread is blocked
    EXPECT_FALSE(thread_woken.load(std::memory_order_acquire));

    // Wake it
    futex_word.store(1, std::memory_order_release);
    futex_wake_all(&futex_word);

    waiter.join();
    EXPECT_TRUE(thread_woken.load(std::memory_order_acquire));
}

TEST_F(FutexTest, test_futex_timeout) {
    std::atomic<uint32_t> futex_word{0};

    auto start = std::chrono::steady_clock::now();
    bool result = futex_wait(&futex_word, 0, 50ms);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);  // timed out
    EXPECT_GE(elapsed, 50ms);
    EXPECT_LT(elapsed, 150ms);  // didn't wait too long
}

TEST_F(FutexTest, test_futex_already_changed) {
    std::atomic<uint32_t> futex_word{1};

    // Expected value doesn't match, should return immediately
    auto start = std::chrono::steady_clock::now();
    bool result = futex_wait(&futex_word, 0);  // expect 0, but it's 1
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(result);  // returns immediately (EAGAIN)
    EXPECT_LT(elapsed, 10ms);
}

TEST_F(FutexTest, test_futex_wake_count) {
    std::atomic<uint32_t> futex_word{0};
    std::atomic<int> woken_count{0};
    constexpr int NUM_WAITERS = 4;

    std::vector<std::thread> waiters;
    std::atomic<int> started{0};

    for (int i = 0; i < NUM_WAITERS; ++i) {
        waiters.emplace_back([&]() {
            started.fetch_add(1, std::memory_order_release);
            futex_wait(&futex_word, 0);
            woken_count.fetch_add(1, std::memory_order_release);
        });
    }

    // Wait for all threads to start
    while (started.load(std::memory_order_acquire) < NUM_WAITERS) {
        std::this_thread::sleep_for(1ms);
    }
    std::this_thread::sleep_for(10ms);

    // Wake just one
    futex_word.store(1, std::memory_order_release);
    futex_wake(&futex_word, 1);

    std::this_thread::sleep_for(20ms);

    // Wake all remaining
    futex_word.store(2, std::memory_order_release);
    futex_wake_all(&futex_word);

    for (auto& t : waiters) {
        t.join();
    }

    EXPECT_EQ(woken_count.load(), NUM_WAITERS);
}

TEST_F(FutexTest, test_ring_buffer_wait) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    // Reset read index to read from beginning
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    std::atomic<bool> received{false};

    // Reader thread
    std::thread reader_thread([&]() {
        auto result = reader.wait(slot);  // blocks
        EXPECT_TRUE(result.has_value());
        received.store(true, std::memory_order_release);
    });

    std::this_thread::sleep_for(10ms);
    EXPECT_FALSE(received.load(std::memory_order_acquire));  // still waiting

    // Write data
    writer.try_write("hello", 5);

    reader_thread.join();
    EXPECT_TRUE(received.load(std::memory_order_acquire));
}

TEST_F(FutexTest, test_ring_buffer_wait_timeout) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();

    // Wait with timeout, no data
    auto start = std::chrono::steady_clock::now();
    auto result = reader.wait_for(slot, 50ms);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result.has_value());
    EXPECT_GE(elapsed, 50ms);
}

TEST_F(FutexTest, test_ring_buffer_wait_immediate) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    // Reset read index to read from beginning
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    // Write data first
    writer.try_write("hello", 5);

    // Wait should return immediately
    auto start = std::chrono::steady_clock::now();
    auto result = reader.wait(slot);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(result.has_value());
    EXPECT_LT(elapsed, 10ms);
    EXPECT_EQ(result->size, 5u);
}

TEST_F(FutexTest, test_ring_buffer_wait_for_with_data) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    // Reset read index
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    std::atomic<bool> received{false};

    // Reader thread with timeout
    std::thread reader_thread([&]() {
        auto result = reader.wait_for(slot, 500ms);
        if (result.has_value()) {
            received.store(true, std::memory_order_release);
        }
    });

    std::this_thread::sleep_for(20ms);

    // Write data before timeout
    writer.try_write("data", 4);

    reader_thread.join();
    EXPECT_TRUE(received.load(std::memory_order_acquire));
}
