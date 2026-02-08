#include "conduit_types/primitives/int.hpp"
#include "conduit_types/primitives/double.hpp"
#include "conduit_types/primitives/bool.hpp"
#include "conduit_types/primitives/uint.hpp"
#include "conduit_types/primitives/time.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <limits>
#include <vector>

using namespace conduit;

// Compile-time validation for all primitive types
template void validate_fixed_message_type<Int>();
template void validate_fixed_message_type<Double>();
template void validate_fixed_message_type<Bool>();
template void validate_fixed_message_type<Uint>();
template void validate_fixed_message_type<Time>();

class PrimitivesTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(PrimitivesTest, test_int_roundtrip) {
    Int original{};
    original.value = -42;

    std::vector<uint8_t> buffer(sizeof(Int));
    std::memcpy(buffer.data(), &original, sizeof(Int));

    Int received{};
    std::memcpy(&received, buffer.data(), sizeof(Int));
    EXPECT_EQ(received.value, -42);
}

TEST_F(PrimitivesTest, test_int_limits) {
    Int msg{};

    msg.value = INT64_MAX;
    EXPECT_EQ(msg.value, INT64_MAX);

    msg.value = INT64_MIN;
    EXPECT_EQ(msg.value, INT64_MIN);

    msg.value = 0;
    EXPECT_EQ(msg.value, 0);
}

TEST_F(PrimitivesTest, test_double_roundtrip) {
    Double original{};
    original.value = 3.14159265358979;

    std::vector<uint8_t> buffer(sizeof(Double));
    std::memcpy(buffer.data(), &original, sizeof(Double));

    Double received{};
    std::memcpy(&received, buffer.data(), sizeof(Double));
    EXPECT_DOUBLE_EQ(received.value, 3.14159265358979);
}

TEST_F(PrimitivesTest, test_double_special_values) {
    Double msg{};

    msg.value = 0.0;
    EXPECT_DOUBLE_EQ(msg.value, 0.0);

    msg.value = -0.0;
    EXPECT_DOUBLE_EQ(msg.value, -0.0);

    msg.value = std::numeric_limits<double>::infinity();
    EXPECT_TRUE(std::isinf(msg.value));

    msg.value = std::numeric_limits<double>::quiet_NaN();
    EXPECT_TRUE(std::isnan(msg.value));
}

TEST_F(PrimitivesTest, test_bool_roundtrip) {
    Bool original{};
    original.value = true;

    std::vector<uint8_t> buffer(sizeof(Bool));
    std::memcpy(buffer.data(), &original, sizeof(Bool));

    Bool received{};
    std::memcpy(&received, buffer.data(), sizeof(Bool));
    EXPECT_TRUE(received.value);
}

TEST_F(PrimitivesTest, test_bool_false) {
    Bool msg{};
    msg.value = false;
    EXPECT_FALSE(msg.value);
}

TEST_F(PrimitivesTest, test_uint_roundtrip) {
    Uint original{};
    original.value = 1234567890123456789ULL;

    std::vector<uint8_t> buffer(sizeof(Uint));
    std::memcpy(buffer.data(), &original, sizeof(Uint));

    Uint received{};
    std::memcpy(&received, buffer.data(), sizeof(Uint));
    EXPECT_EQ(received.value, 1234567890123456789ULL);
}

TEST_F(PrimitivesTest, test_uint_max) {
    Uint msg{};
    msg.value = UINT64_MAX;
    EXPECT_EQ(msg.value, UINT64_MAX);
}

TEST_F(PrimitivesTest, test_time_roundtrip) {
    Time original{};
    original.nanoseconds = 1700000000000000000ULL;  // ~2023 in ns

    std::vector<uint8_t> buffer(sizeof(Time));
    std::memcpy(buffer.data(), &original, sizeof(Time));

    Time received{};
    std::memcpy(&received, buffer.data(), sizeof(Time));
    EXPECT_EQ(received.nanoseconds, 1700000000000000000ULL);
}

TEST_F(PrimitivesTest, test_time_zero) {
    Time msg{};
    msg.nanoseconds = 0;
    EXPECT_EQ(msg.nanoseconds, 0u);
}
