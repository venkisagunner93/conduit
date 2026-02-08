#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/variable_message_type.hpp"

#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

using namespace conduit;

// --- Fixed message type examples ---

struct ImuMessage : public FixedMessageType {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    uint64_t timestamp_ns;
};

// Compile-time validation — if this compiles, the checks pass
static_assert(std::is_trivially_copyable_v<ImuMessage>);
static_assert(std::is_standard_layout_v<ImuMessage>);

// --- Variable message type examples ---

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

class TypesTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(TypesTest, test_fixed_message_type_validation) {
    // This call validates at compile time via static_assert.
    // If it compiles, the type is valid.
    validate_fixed_message_type<ImuMessage>();
}

TEST_F(TypesTest, test_fixed_message_type_memcpy_roundtrip) {
    ImuMessage original{};
    original.accel_x = 1.0;
    original.accel_y = 2.0;
    original.accel_z = 9.81;
    original.gyro_x = 0.1;
    original.gyro_y = 0.2;
    original.gyro_z = 0.3;
    original.timestamp_ns = 1234567890;

    // Simulate publish: memcpy to buffer
    std::vector<uint8_t> buffer(sizeof(ImuMessage));
    std::memcpy(buffer.data(), &original, sizeof(ImuMessage));

    // Simulate subscribe: memcpy from buffer
    ImuMessage received{};
    std::memcpy(&received, buffer.data(), sizeof(ImuMessage));

    EXPECT_DOUBLE_EQ(received.accel_x, 1.0);
    EXPECT_DOUBLE_EQ(received.accel_y, 2.0);
    EXPECT_DOUBLE_EQ(received.accel_z, 9.81);
    EXPECT_DOUBLE_EQ(received.gyro_x, 0.1);
    EXPECT_DOUBLE_EQ(received.gyro_y, 0.2);
    EXPECT_DOUBLE_EQ(received.gyro_z, 0.3);
    EXPECT_EQ(received.timestamp_ns, 1234567890u);
}

TEST_F(TypesTest, test_variable_message_type_validation) {
    // This call validates at compile time via static_assert.
    // If it compiles, the type is valid.
    validate_variable_message_type<StringMessage>();
}

TEST_F(TypesTest, test_variable_message_type_serialize_roundtrip) {
    StringMessage original("hello conduit");

    // Serialize
    size_t size = original.serialized_size();
    EXPECT_EQ(size, sizeof(uint32_t) + 13);

    std::vector<uint8_t> buffer(size);
    original.serialize(buffer.data());

    // Deserialize
    StringMessage received = StringMessage::deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(received.text, "hello conduit");
}

TEST_F(TypesTest, test_variable_message_type_empty_string) {
    StringMessage original("");

    size_t size = original.serialized_size();
    EXPECT_EQ(size, sizeof(uint32_t));

    std::vector<uint8_t> buffer(size);
    original.serialize(buffer.data());

    StringMessage received = StringMessage::deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(received.text, "");
}
