#include "conduit_core/publisher.hpp"
#include "conduit_core/subscriber.hpp"
#include "conduit_core/exceptions.hpp"
#include "conduit_core/internal/shm_region.hpp"
#include "conduit_core/internal/time.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

using namespace conduit;
using namespace std::chrono_literals;

class PubSubTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any test topics
        for (int i = 1; i <= 10; ++i) {
            internal::ShmRegion::unlink("test_topic_" + std::to_string(i));
        }
    }
};

TEST_F(PubSubTest, test_publish_subscribe_basic) {
    const std::string topic = "test_topic_1";

    // Create publisher
    Publisher pub(topic, {.depth = 16, .max_message_size = 1024});

    // Create subscriber
    Subscriber sub(topic);

    // Publish
    const char* data = "hello world";
    ASSERT_TRUE(pub.publish(data, std::strlen(data)));

    // Receive
    auto msg = sub.take();
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->size, std::strlen(data));
    EXPECT_EQ(std::memcmp(msg->data, data, msg->size), 0);
}

TEST_F(PubSubTest, test_publish_subscribe_wait) {
    const std::string topic = "test_topic_2";

    Publisher pub(topic);
    Subscriber sub(topic);

    std::atomic<bool> received{false};

    // Subscriber thread
    std::thread sub_thread([&]() {
        auto msg = sub.wait();  // blocks
        received.store(true, std::memory_order_release);
        EXPECT_EQ(msg.size, 5u);
    });

    std::this_thread::sleep_for(10ms);
    EXPECT_FALSE(received.load(std::memory_order_acquire));

    pub.publish("hello", 5);

    sub_thread.join();
    EXPECT_TRUE(received.load(std::memory_order_acquire));
}

TEST_F(PubSubTest, test_publish_subscribe_timeout) {
    const std::string topic = "test_topic_3";

    Publisher pub(topic);
    Subscriber sub(topic);

    auto start = std::chrono::steady_clock::now();
    auto msg = sub.wait_for(50ms);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(msg.has_value());
    EXPECT_GE(elapsed, 50ms);
}

TEST_F(PubSubTest, test_multiple_subscribers) {
    const std::string topic = "test_topic_4";

    Publisher pub(topic);
    Subscriber sub1(topic);
    Subscriber sub2(topic);
    Subscriber sub3(topic);

    pub.publish("message", 7);

    auto msg1 = sub1.take();
    auto msg2 = sub2.take();
    auto msg3 = sub3.take();

    ASSERT_TRUE(msg1.has_value());
    ASSERT_TRUE(msg2.has_value());
    ASSERT_TRUE(msg3.has_value());

    EXPECT_EQ(msg1->size, 7u);
    EXPECT_EQ(msg2->size, 7u);
    EXPECT_EQ(msg3->size, 7u);
}

TEST_F(PubSubTest, test_publisher_message_too_large) {
    const std::string topic = "test_topic_5";

    Publisher pub(topic, {.depth = 16, .max_message_size = 100});

    char big_data[200];
    std::memset(big_data, 'x', sizeof(big_data));
    EXPECT_FALSE(pub.publish(big_data, sizeof(big_data)));

    char small_data[50];
    std::memset(small_data, 'y', sizeof(small_data));
    EXPECT_TRUE(pub.publish(small_data, sizeof(small_data)));
}

TEST_F(PubSubTest, test_subscriber_before_publisher) {
    const std::string topic = "test_topic_6";

    // Ensure clean state
    internal::ShmRegion::unlink(topic);

    // Subscriber without publisher should throw
    EXPECT_THROW(Subscriber sub(topic), ShmError);
}

TEST_F(PubSubTest, test_subscriber_max_limit) {
    const std::string topic = "test_topic_7";

    Publisher pub(topic);

    std::vector<std::unique_ptr<Subscriber>> subscribers;
    for (int i = 0; i < 16; ++i) {
        subscribers.push_back(std::make_unique<Subscriber>(topic));
    }

    // 17th should fail
    EXPECT_THROW(Subscriber sub(topic), SubscriberError);
}

TEST_F(PubSubTest, test_publisher_destructor_cleanup) {
    const std::string topic = "test_topic_8";

    {
        Publisher pub(topic);
        EXPECT_TRUE(internal::ShmRegion::exists(topic));
    }
    // Publisher destroyed

    EXPECT_FALSE(internal::ShmRegion::exists(topic));
}

TEST_F(PubSubTest, test_high_throughput) {
    const std::string topic = "test_topic_9";

    Publisher pub(topic, {.depth = 1024, .max_message_size = 64});
    Subscriber sub(topic);

    constexpr int count = 100000;
    std::atomic<int> received{0};

    // Publisher thread
    std::thread pub_thread([&]() {
        for (int i = 0; i < count; ++i) {
            while (!pub.publish(&i, sizeof(i))) {
                std::this_thread::yield();
            }
        }
    });

    // Subscriber thread
    std::thread sub_thread([&]() {
        while (received.load(std::memory_order_acquire) < count) {
            if (auto msg = sub.take()) {
                received.fetch_add(1, std::memory_order_release);
            } else {
                std::this_thread::yield();
            }
        }
    });

    pub_thread.join();
    sub_thread.join();

    EXPECT_EQ(received.load(), count);
}

TEST_F(PubSubTest, test_sequence_numbers) {
    const std::string topic = "test_topic_10";

    Publisher pub(topic, {.depth = 16, .max_message_size = 64});
    Subscriber sub(topic);

    // Publish multiple messages
    for (int i = 0; i < 5; ++i) {
        pub.publish(&i, sizeof(i));
    }

    // Check sequence numbers are incrementing
    uint64_t last_seq = 0;
    for (int i = 0; i < 5; ++i) {
        auto msg = sub.take();
        ASSERT_TRUE(msg.has_value());
        if (i > 0) {
            EXPECT_EQ(msg->sequence, last_seq + 1);
        }
        last_seq = msg->sequence;
    }
}

TEST_F(PubSubTest, test_accessors) {
    const std::string topic = "test_topic_1";

    Publisher pub(topic, {.depth = 32, .max_message_size = 2048});
    Subscriber sub(topic);

    EXPECT_EQ(pub.topic(), topic);
    EXPECT_EQ(pub.max_message_size(), 2048u);
    EXPECT_EQ(sub.topic(), topic);
}

TEST_F(PubSubTest, test_move_semantics) {
    const std::string topic = "test_topic_1";

    Publisher pub1(topic);

    // Create subscriber before publishing
    Subscriber sub1(topic);

    // Move publisher and publish
    Publisher pub2 = std::move(pub1);
    EXPECT_EQ(pub2.topic(), topic);
    pub2.publish("test", 4);

    // Move subscriber and read
    Subscriber sub2 = std::move(sub1);

    auto msg = sub2.take();
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->size, 4u);
}

TEST_F(PubSubTest, test_message_has_timestamp) {
    const std::string topic = "test_topic_1";

    Publisher pub(topic);
    Subscriber sub(topic);

    uint64_t before = internal::get_timestamp_ns();
    pub.publish("hello", 5);
    uint64_t after = internal::get_timestamp_ns();

    auto msg = sub.take();

    ASSERT_TRUE(msg.has_value());
    EXPECT_GE(msg->timestamp_ns, before);
    EXPECT_LE(msg->timestamp_ns, after);
}

TEST_F(PubSubTest, test_timestamps_increasing) {
    const std::string topic = "test_topic_1";

    Publisher pub(topic);
    Subscriber sub(topic);

    pub.publish("one", 3);
    pub.publish("two", 3);
    pub.publish("three", 5);

    auto msg1 = sub.take();
    auto msg2 = sub.take();
    auto msg3 = sub.take();

    ASSERT_TRUE(msg1.has_value());
    ASSERT_TRUE(msg2.has_value());
    ASSERT_TRUE(msg3.has_value());

    EXPECT_LE(msg1->timestamp_ns, msg2->timestamp_ns);
    EXPECT_LE(msg2->timestamp_ns, msg3->timestamp_ns);
}

TEST_F(PubSubTest, test_sequence_starts_at_zero) {
    const std::string topic = "test_topic_1";

    Publisher pub(topic);
    Subscriber sub(topic);

    pub.publish("a", 1);
    pub.publish("b", 1);
    pub.publish("c", 1);

    auto msg1 = sub.take();
    auto msg2 = sub.take();
    auto msg3 = sub.take();

    ASSERT_TRUE(msg1.has_value());
    ASSERT_TRUE(msg2.has_value());
    ASSERT_TRUE(msg3.has_value());

    EXPECT_EQ(msg1->sequence, 0u);
    EXPECT_EQ(msg2->sequence, 1u);
    EXPECT_EQ(msg3->sequence, 2u);
}

TEST_F(PubSubTest, test_gap_detection) {
    const std::string topic = "test_topic_1";

    Publisher pub(topic, {.depth = 4, .max_message_size = 64});
    Subscriber sub(topic);

    // Publish more than depth (causes overwrites)
    for (int i = 0; i < 10; ++i) {
        pub.publish(&i, sizeof(i));
    }

    // Read what's available
    std::vector<Message> messages;
    while (auto msg = sub.take()) {
        messages.push_back(*msg);
    }

    // Should have at most 4 messages (buffer depth)
    EXPECT_LE(messages.size(), 4u);

    // If we got messages, they should be from the later sequence numbers
    if (!messages.empty()) {
        // First message should be at least sequence 6 (10 - 4 = 6)
        EXPECT_GE(messages[0].sequence, 6u);
    }
}
