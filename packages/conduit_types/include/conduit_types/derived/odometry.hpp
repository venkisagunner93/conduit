#pragma once

#include "conduit_types/derived/pose3d.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

/// @brief Odometry message with pose and velocity.
struct Odometry : FixedMessageType {
    Header header;              ///< Timestamp and coordinate frame.
    char child_frame[64];       ///< Child coordinate frame (null-terminated).
    Pose3D pose;                ///< 3D pose estimate.
    Vec3 linear_velocity;       ///< Linear velocity (m/s).
    Vec3 angular_velocity;      ///< Angular velocity (rad/s).
};

}  // namespace conduit
