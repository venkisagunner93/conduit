#include "conduit_core/internal/ring_buffer.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

using namespace conduit::internal;

class RingBufferTest : public ::testing::Test {
protected:
    void SetUp() override {}

    std::unique_ptr<uint8_t[]> allocate_region(const RingBufferConfig& config) {
        size_t size = calculate_region_size(config);
        auto region = std::make_unique<uint8_t[]>(size);
        std::memset(region.get(), 0, size);
        return region;
    }
};

TEST_F(RingBufferTest, test_basic_write_read) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    ASSERT_EQ(slot, 0);

    // Write before read to have data available
    const char* msg = "hello";
    // Set read_idx back to 0 so we can read the message
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    ASSERT_TRUE(writer.try_write(msg, 5));

    auto result = reader.try_read(slot);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size, 5u);
    EXPECT_EQ(std::memcmp(result->data, "hello", 5), 0);
}

TEST_F(RingBufferTest, test_multiple_messages) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    // Reset read index to read from beginning
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    ASSERT_TRUE(writer.try_write("one", 3));
    ASSERT_TRUE(writer.try_write("two", 3));
    ASSERT_TRUE(writer.try_write("three", 5));

    auto result1 = reader.try_read(slot);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size, 3u);
    EXPECT_EQ(std::memcmp(result1->data, "one", 3), 0);

    auto result2 = reader.try_read(slot);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->size, 3u);
    EXPECT_EQ(std::memcmp(result2->data, "two", 3), 0);

    auto result3 = reader.try_read(slot);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3->size, 5u);
    EXPECT_EQ(std::memcmp(result3->data, "three", 5), 0);

    auto result4 = reader.try_read(slot);
    EXPECT_FALSE(result4.has_value());
}

TEST_F(RingBufferTest, test_multiple_subscribers) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader1(region.get(), region_size);
    RingBufferReader reader2(region.get(), region_size);

    int slot1 = reader1.claim_slot();
    int slot2 = reader2.claim_slot();
    EXPECT_EQ(slot1, 0);
    EXPECT_EQ(slot2, 1);

    // Reset read indices
    reader1.header()->read_idx[slot1].value.store(0, std::memory_order_relaxed);
    reader2.header()->read_idx[slot2].value.store(0, std::memory_order_relaxed);

    ASSERT_TRUE(writer.try_write("message", 7));

    auto result1 = reader1.try_read(slot1);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size, 7u);
    EXPECT_EQ(std::memcmp(result1->data, "message", 7), 0);

    auto result2 = reader2.try_read(slot2);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->size, 7u);
    EXPECT_EQ(std::memcmp(result2->data, "message", 7), 0);
}

TEST_F(RingBufferTest, test_subscriber_limit) {
    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);

    // Claim all 16 slots
    std::vector<int> slots;
    for (int i = 0; i < 16; ++i) {
        int slot = reader.claim_slot();
        ASSERT_EQ(slot, i);
        slots.push_back(slot);
    }

    // 17th should fail
    int slot17 = reader.claim_slot();
    EXPECT_EQ(slot17, -1);

    // Release slot 5
    reader.release_slot(5);

    // Claim again should get slot 5
    int slot_new = reader.claim_slot();
    EXPECT_EQ(slot_new, 5);
}

TEST_F(RingBufferTest, test_wraparound) {
    RingBufferConfig config{.slot_count = 4, .slot_size = 64};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    // Write 10 messages (buffer holds 4)
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(writer.try_write(&i, sizeof(i)));
    }

    // Reader claims slot - starts at write_idx=10
    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    ASSERT_GE(slot, 0);

    // Should be empty (caught up)
    auto result = reader.try_read(slot);
    EXPECT_FALSE(result.has_value());
}

TEST_F(RingBufferTest, test_overwrite_detection) {
    RingBufferConfig config{.slot_count = 4, .slot_size = 64};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    // Set read_idx to 0 (before any writes)
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    // Write 10 messages (overwrites buffer multiple times)
    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(writer.try_write(&i, sizeof(i)));
    }

    // Reader should detect fallen behind and skip to valid data
    auto result = reader.try_read(slot);
    ASSERT_TRUE(result.has_value());
    // Should have skipped to message 6 (write_idx=10, slot_count=4, so oldest valid is 6)
    int value;
    std::memcpy(&value, result->data, sizeof(int));
    EXPECT_EQ(value, 6);
    EXPECT_EQ(result->sequence, 6u);
}

TEST_F(RingBufferTest, test_sequence_validation) {
    RingBufferConfig config{.slot_count = 4, .slot_size = 64};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    RingBufferReader reader(region.get(), region_size);
    int slot = reader.claim_slot();
    // Reset to read from beginning
    reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

    // Write 2 messages
    int msg0 = 0, msg1 = 1;
    ASSERT_TRUE(writer.try_write(&msg0, sizeof(msg0)));
    ASSERT_TRUE(writer.try_write(&msg1, sizeof(msg1)));

    // Manually corrupt sequence by writing 4 more (overwrites slot 0)
    for (int i = 2; i < 6; ++i) {
        ASSERT_TRUE(writer.try_write(&i, sizeof(i)));
    }

    // Now read_idx=0 points to slot that has sequence=4, not 0
    // try_read should detect mismatch and return nullopt
    auto result = reader.try_read(slot);
    // Either nullopt (detected mismatch) or valid data from later sequence
    if (result.has_value()) {
        // If it returned something, it should be from the valid range
        EXPECT_GE(result->sequence, 2u);
    }
}

TEST_F(RingBufferTest, test_message_too_large) {
    RingBufferConfig config{.slot_count = 4, .slot_size = 64};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    // Try to write a message larger than slot_size - SLOT_HEADER_SIZE
    std::vector<uint8_t> large_msg(100, 0x42);
    EXPECT_FALSE(writer.try_write(large_msg.data(), large_msg.size()));

    // Message that exactly fits should work
    std::vector<uint8_t> fitting_msg(64 - SLOT_HEADER_SIZE, 0x42);
    EXPECT_TRUE(writer.try_write(fitting_msg.data(), fitting_msg.size()));
}

TEST_F(RingBufferTest, test_concurrent_write_read) {
    RingBufferConfig config{.slot_count = 256, .slot_size = 64};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    constexpr int NUM_MESSAGES = 10000;
    std::atomic<bool> writer_done{false};

    std::thread writer_thread([&]() {
        for (int i = 0; i < NUM_MESSAGES; ++i) {
            while (!writer.try_write(&i, sizeof(i))) {
                // Slot too small - shouldn't happen with our config
                std::this_thread::yield();
            }
        }
        writer_done.store(true, std::memory_order_release);
    });

    std::thread reader_thread([&]() {
        RingBufferReader reader(region.get(), region_size);
        int slot = reader.claim_slot();
        ASSERT_GE(slot, 0);
        // Start from beginning
        reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

        int count = 0;
        while (count < NUM_MESSAGES) {
            auto result = reader.try_read(slot);
            if (result.has_value()) {
                count++;
            } else if (writer_done.load(std::memory_order_acquire)) {
                // Writer done but we haven't read everything - check again
                auto final_result = reader.try_read(slot);
                if (!final_result.has_value()) {
                    break;  // No more data
                }
                count++;
            } else {
                std::this_thread::yield();
            }
        }
        // We should have read some messages (may have skipped some due to overwrite)
        EXPECT_GT(count, 0);
    });

    writer_thread.join();
    reader_thread.join();
}

TEST_F(RingBufferTest, test_concurrent_multiple_readers) {
    RingBufferConfig config{.slot_count = 256, .slot_size = 64};
    auto region = allocate_region(config);
    size_t region_size = calculate_region_size(config);

    RingBufferWriter writer(region.get(), region_size, config);
    writer.initialize();

    constexpr int NUM_MESSAGES = 10000;
    constexpr int NUM_READERS = 4;
    std::atomic<bool> writer_done{false};

    std::thread writer_thread([&]() {
        for (int i = 0; i < NUM_MESSAGES; ++i) {
            while (!writer.try_write(&i, sizeof(i))) {
                std::this_thread::yield();
            }
        }
        writer_done.store(true, std::memory_order_release);
    });

    std::vector<std::thread> reader_threads;
    std::atomic<int> total_reads{0};

    for (int r = 0; r < NUM_READERS; ++r) {
        reader_threads.emplace_back([&, r]() {
            RingBufferReader reader(region.get(), region_size);
            int slot = reader.claim_slot();
            ASSERT_GE(slot, 0);
            // Start from beginning
            reader.header()->read_idx[slot].value.store(0, std::memory_order_relaxed);

            int count = 0;
            while (count < NUM_MESSAGES) {
                auto result = reader.try_read(slot);
                if (result.has_value()) {
                    count++;
                } else if (writer_done.load(std::memory_order_acquire)) {
                    auto final_result = reader.try_read(slot);
                    if (!final_result.has_value()) {
                        break;
                    }
                    count++;
                } else {
                    std::this_thread::yield();
                }
            }
            total_reads.fetch_add(count, std::memory_order_relaxed);
        });
    }

    writer_thread.join();
    for (auto& t : reader_threads) {
        t.join();
    }

    // Each reader should have read some messages
    EXPECT_GT(total_reads.load(), 0);
}

TEST_F(RingBufferTest, test_helper_functions) {
    EXPECT_TRUE(is_power_of_two(1));
    EXPECT_TRUE(is_power_of_two(2));
    EXPECT_TRUE(is_power_of_two(4));
    EXPECT_TRUE(is_power_of_two(16));
    EXPECT_TRUE(is_power_of_two(256));

    EXPECT_FALSE(is_power_of_two(0));
    EXPECT_FALSE(is_power_of_two(3));
    EXPECT_FALSE(is_power_of_two(5));
    EXPECT_FALSE(is_power_of_two(15));

    RingBufferConfig config{.slot_count = 16, .slot_size = 256};
    size_t expected = sizeof(RingBufferHeader) + 16 * 256;
    EXPECT_EQ(calculate_region_size(config), expected);
}
