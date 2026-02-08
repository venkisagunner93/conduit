#include "conduit_core/publisher.hpp"
#include "conduit_core/subscriber.hpp"
#include "conduit_core/internal/shm_region.hpp"

#include <conduit_types/primitives/int.hpp>
#include <conduit_types/primitives/double.hpp>
#include <conduit_types/primitives/bool.hpp>
#include <conduit_types/primitives/uint.hpp>
#include <conduit_types/primitives/time.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>

using namespace conduit;
using namespace std::chrono_literals;

// --- Variable message type for testing ---

struct StringMessage : public VariableMessageType {
    std::string text;

    StringMessage() = default;
    explicit StringMessage(std::string t) : text(std::move(t)) {}

    size_t serialized_size() const override {
        return sizeof(uint32_t) + text.size();
    }

    void serialize(uint8_t* buffer) const override {
        uint32_t len = static_cast<uint32_t>(text.size());
        std::memcpy(buffer, &len, sizeof(len));
        std::memcpy(buffer + sizeof(len), text.data(), text.size());
    }

    static StringMessage deserialize(const uint8_t* data, size_t size) {
        uint32_t len;
        std::memcpy(&len, data, sizeof(len));
        return StringMessage(std::string(reinterpret_cast<const char*>(data + sizeof(len)), len));
    }
};

// --- Tests ---

class TypedPubSubTest : public ::testing::Test {
protected:
    void TearDown() override {
        for (int i = 1; i <= 10; ++i) {
            internal::ShmRegion::unlink("typed_test_" + std::to_string(i));
        }
    }
};

TEST_F(TypedPubSubTest, test_fixed_int_roundtrip) {
    const std::string topic = "typed_test_1";

    Publisher<Int> pub(topic);
    Subscriber<Int> sub(topic);

    Int msg{};
    msg.value = 42;
    ASSERT_TRUE(pub.publish(msg));

    auto received = sub.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->data.value, 42);
}

TEST_F(TypedPubSubTest, test_fixed_float_roundtrip) {
    const std::string topic = "typed_test_2";

    Publisher<Double> pub(topic);
    Subscriber<Double> sub(topic);

    Double msg{};
    msg.value = 3.14;
    ASSERT_TRUE(pub.publish(msg));

    auto received = sub.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_DOUBLE_EQ(received->data.value, 3.14);
}

TEST_F(TypedPubSubTest, test_fixed_bool_roundtrip) {
    const std::string topic = "typed_test_3";

    Publisher<Bool> pub(topic);
    Subscriber<Bool> sub(topic);

    Bool msg{};
    msg.value = true;
    ASSERT_TRUE(pub.publish(msg));

    auto received = sub.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_TRUE(received->data.value);
}

TEST_F(TypedPubSubTest, test_fixed_uint_roundtrip) {
    const std::string topic = "typed_test_4";

    Publisher<Uint> pub(topic);
    Subscriber<Uint> sub(topic);

    Uint msg{};
    msg.value = 1234567890ULL;
    ASSERT_TRUE(pub.publish(msg));

    auto received = sub.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->data.value, 1234567890ULL);
}

TEST_F(TypedPubSubTest, test_fixed_time_roundtrip) {
    const std::string topic = "typed_test_5";

    Publisher<Time> pub(topic);
    Subscriber<Time> sub(topic);

    Time msg{};
    msg.nanoseconds = 1700000000000000000ULL;
    ASSERT_TRUE(pub.publish(msg));

    auto received = sub.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->data.nanoseconds, 1700000000000000000ULL);
}

TEST_F(TypedPubSubTest, test_variable_string_roundtrip) {
    const std::string topic = "typed_test_6";

    Publisher<StringMessage> pub(topic);
    Subscriber<StringMessage> sub(topic);

    StringMessage msg("hello typed conduit");
    ASSERT_TRUE(pub.publish(msg));

    auto received = sub.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->data.text, "hello typed conduit");
}

TEST_F(TypedPubSubTest, test_metadata_preserved) {
    const std::string topic = "typed_test_7";

    Publisher<Int> pub(topic);
    Subscriber<Int> sub(topic);

    Int msg{};
    msg.value = 1;
    pub.publish(msg);

    msg.value = 2;
    pub.publish(msg);

    auto r1 = sub.take();
    auto r2 = sub.take();

    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r2.has_value());

    EXPECT_EQ(r1->sequence, 0u);
    EXPECT_EQ(r2->sequence, 1u);
    EXPECT_LE(r1->timestamp_ns, r2->timestamp_ns);
}

TEST_F(TypedPubSubTest, test_typed_wait_for) {
    const std::string topic = "typed_test_8";

    Publisher<Int> pub(topic);
    Subscriber<Int> sub(topic);

    // Should timeout with no data
    auto result = sub.wait_for(10ms);
    EXPECT_FALSE(result.has_value());

    // Publish and read
    Int msg{};
    msg.value = 99;
    pub.publish(msg);

    result = sub.wait_for(100ms);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->data.value, 99);
}

TEST_F(TypedPubSubTest, test_typed_wait) {
    const std::string topic = "typed_test_9";

    Publisher<Int> pub(topic);
    Subscriber<Int> sub(topic);

    std::thread pub_thread([&]() {
        std::this_thread::sleep_for(10ms);
        Int msg{};
        msg.value = 77;
        pub.publish(msg);
    });

    auto received = sub.wait();
    EXPECT_EQ(received.data.value, 77);

    pub_thread.join();
}

TEST_F(TypedPubSubTest, test_multiple_messages) {
    const std::string topic = "typed_test_10";

    Publisher<Int> pub(topic, {.depth = 128, .max_message_size = 64});
    Subscriber<Int> sub(topic);

    for (int64_t i = 0; i < 100; ++i) {
        Int msg{};
        msg.value = i;
        ASSERT_TRUE(pub.publish(msg));
    }

    for (int64_t i = 0; i < 100; ++i) {
        auto received = sub.take();
        ASSERT_TRUE(received.has_value());
        EXPECT_EQ(received->data.value, i);
    }
}
