#include "conduit_types/primitives/vec2.hpp"
#include "conduit_types/primitives/vec3.hpp"
#include "conduit_types/derived/orientation.hpp"
#include "conduit_types/derived/pose2d.hpp"
#include "conduit_types/derived/pose3d.hpp"
#include "conduit_types/derived/twist.hpp"
#include "conduit_types/derived/imu.hpp"
#include "conduit_types/derived/odometry.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <vector>

using namespace conduit;

// Compile-time validation
template void validate_fixed_message_type<Vec2>();
template void validate_fixed_message_type<Vec3>();
template void validate_fixed_message_type<Orientation>();
template void validate_fixed_message_type<Pose2D>();
template void validate_fixed_message_type<Pose3D>();
template void validate_fixed_message_type<Twist>();
template void validate_fixed_message_type<Imu>();
template void validate_fixed_message_type<Odometry>();

// --- Orientation conversions ---

TEST(OrientationTest, from_yaw_to_yaw_roundtrip) {
    double yaw = 1.57;
    auto q = Orientation::from_yaw(yaw);
    EXPECT_NEAR(q.to_yaw(), yaw, 1e-10);
}

TEST(OrientationTest, from_yaw_identity) {
    auto q = Orientation::from_yaw(0.0);
    EXPECT_NEAR(q.x, 0.0, 1e-10);
    EXPECT_NEAR(q.y, 0.0, 1e-10);
    EXPECT_NEAR(q.z, 0.0, 1e-10);
    EXPECT_NEAR(q.w, 1.0, 1e-10);
}

TEST(OrientationTest, from_yaw_90deg) {
    auto q = Orientation::from_yaw(M_PI / 2);
    EXPECT_NEAR(q.x, 0.0, 1e-10);
    EXPECT_NEAR(q.y, 0.0, 1e-10);
    EXPECT_NEAR(q.z, std::sin(M_PI / 4), 1e-10);
    EXPECT_NEAR(q.w, std::cos(M_PI / 4), 1e-10);
}

TEST(OrientationTest, euler_zyx_roundtrip) {
    double roll = 0.3, pitch = 0.2, yaw = 1.0;
    auto q = Orientation::from_euler(roll, pitch, yaw, EulerOrder::ZYX);
    auto e = q.to_euler(EulerOrder::ZYX);
    EXPECT_NEAR(e.x, roll, 1e-10);
    EXPECT_NEAR(e.y, pitch, 1e-10);
    EXPECT_NEAR(e.z, yaw, 1e-10);
}

TEST(OrientationTest, euler_xyz_roundtrip) {
    double roll = 0.1, pitch = -0.4, yaw = 0.8;
    auto q = Orientation::from_euler(roll, pitch, yaw, EulerOrder::XYZ);
    auto e = q.to_euler(EulerOrder::XYZ);
    EXPECT_NEAR(e.x, roll, 1e-10);
    EXPECT_NEAR(e.y, pitch, 1e-10);
    EXPECT_NEAR(e.z, yaw, 1e-10);
}

TEST(OrientationTest, euler_identity) {
    auto q = Orientation::from_euler(0, 0, 0);
    EXPECT_NEAR(q.x, 0.0, 1e-10);
    EXPECT_NEAR(q.y, 0.0, 1e-10);
    EXPECT_NEAR(q.z, 0.0, 1e-10);
    EXPECT_NEAR(q.w, 1.0, 1e-10);
}

TEST(OrientationTest, euler_pure_roll) {
    double roll = 0.5;
    auto q = Orientation::from_euler(roll, 0, 0);
    auto e = q.to_euler();
    EXPECT_NEAR(e.x, roll, 1e-10);
    EXPECT_NEAR(e.y, 0.0, 1e-10);
    EXPECT_NEAR(e.z, 0.0, 1e-10);
}

TEST(OrientationTest, euler_pure_pitch) {
    double pitch = 0.5;
    auto q = Orientation::from_euler(0, pitch, 0);
    auto e = q.to_euler();
    EXPECT_NEAR(e.x, 0.0, 1e-10);
    EXPECT_NEAR(e.y, pitch, 1e-10);
    EXPECT_NEAR(e.z, 0.0, 1e-10);
}

// --- Pose2D ---

TEST(DerivedTest, pose2d_with_orientation) {
    Pose2D msg{};
    set_frame(msg.header.frame, "map");
    msg.position.x = 5.0;
    msg.position.y = 10.0;
    msg.orientation = Orientation::from_yaw(M_PI / 4);

    EXPECT_NEAR(msg.orientation.to_yaw(), M_PI / 4, 1e-10);

    std::vector<uint8_t> buf(sizeof(msg));
    std::memcpy(buf.data(), &msg, sizeof(msg));

    Pose2D out{};
    std::memcpy(&out, buf.data(), sizeof(out));
    EXPECT_STREQ(out.header.frame, "map");
    EXPECT_DOUBLE_EQ(out.position.x, 5.0);
    EXPECT_NEAR(out.orientation.to_yaw(), M_PI / 4, 1e-10);
}

// --- Pose3D ---

TEST(DerivedTest, pose3d_with_euler) {
    Pose3D msg{};
    set_frame(msg.header.frame, "base_link");
    msg.position.x = 1.0;
    msg.position.y = 2.0;
    msg.position.z = 3.0;
    msg.orientation = Orientation::from_euler(0.1, 0.2, 0.3);

    auto e = msg.orientation.to_euler();
    EXPECT_NEAR(e.x, 0.1, 1e-10);
    EXPECT_NEAR(e.y, 0.2, 1e-10);
    EXPECT_NEAR(e.z, 0.3, 1e-10);

    std::vector<uint8_t> buf(sizeof(msg));
    std::memcpy(buf.data(), &msg, sizeof(msg));

    Pose3D out{};
    std::memcpy(&out, buf.data(), sizeof(out));
    EXPECT_STREQ(out.header.frame, "base_link");
    EXPECT_DOUBLE_EQ(out.position.x, 1.0);
    auto e2 = out.orientation.to_euler();
    EXPECT_NEAR(e2.x, 0.1, 1e-10);
    EXPECT_NEAR(e2.y, 0.2, 1e-10);
    EXPECT_NEAR(e2.z, 0.3, 1e-10);
}

// --- Twist, Imu, Odometry ---

TEST(DerivedTest, twist_composition) {
    Twist msg{};
    set_frame(msg.header.frame, "base_link");
    msg.linear.x = 1.0;
    msg.angular.z = 0.5;

    std::vector<uint8_t> buf(sizeof(msg));
    std::memcpy(buf.data(), &msg, sizeof(msg));

    Twist out{};
    std::memcpy(&out, buf.data(), sizeof(out));
    EXPECT_STREQ(out.header.frame, "base_link");
    EXPECT_DOUBLE_EQ(out.linear.x, 1.0);
    EXPECT_DOUBLE_EQ(out.angular.z, 0.5);
}

TEST(DerivedTest, imu_composition) {
    Imu msg{};
    set_frame(msg.header.frame, "imu_link");
    msg.orientation = Orientation::from_euler(0, 0, 0);
    msg.angular_velocity.z = 0.03;
    msg.linear_acceleration.z = 9.81;

    std::vector<uint8_t> buf(sizeof(msg));
    std::memcpy(buf.data(), &msg, sizeof(msg));

    Imu out{};
    std::memcpy(&out, buf.data(), sizeof(out));
    EXPECT_STREQ(out.header.frame, "imu_link");
    EXPECT_NEAR(out.orientation.w, 1.0, 1e-10);
    EXPECT_DOUBLE_EQ(out.angular_velocity.z, 0.03);
    EXPECT_DOUBLE_EQ(out.linear_acceleration.z, 9.81);
}

TEST(DerivedTest, odometry_composition) {
    Odometry msg{};
    set_frame(msg.header.frame, "odom");
    set_frame(msg.child_frame, "base_link");
    msg.pose.position.x = 10.0;
    msg.pose.orientation = Orientation::from_yaw(0.5);
    msg.linear_velocity.x = 1.5;
    msg.angular_velocity.z = 0.1;

    std::vector<uint8_t> buf(sizeof(msg));
    std::memcpy(buf.data(), &msg, sizeof(msg));

    Odometry out{};
    std::memcpy(&out, buf.data(), sizeof(out));
    EXPECT_STREQ(out.header.frame, "odom");
    EXPECT_STREQ(out.child_frame, "base_link");
    EXPECT_DOUBLE_EQ(out.pose.position.x, 10.0);
    EXPECT_NEAR(out.pose.orientation.to_yaw(), 0.5, 1e-10);
    EXPECT_DOUBLE_EQ(out.linear_velocity.x, 1.5);
    EXPECT_DOUBLE_EQ(out.angular_velocity.z, 0.1);
}

TEST(DerivedTest, frame_truncation) {
    Odometry msg{};
    set_frame(msg.header.frame, "this_is_a_very_long_frame_name_that_exceeds_the_sixty_four_byte_limit!");
    EXPECT_EQ(std::strlen(msg.header.frame), 63u);
}
