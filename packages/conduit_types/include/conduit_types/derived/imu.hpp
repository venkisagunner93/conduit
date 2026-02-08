#pragma once

#include "conduit_types/derived/orientation.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

/// @brief Inertial Measurement Unit data (orientation, angular velocity, linear acceleration).
struct Imu : FixedMessageType {
    Header header;                ///< Timestamp and coordinate frame.
    Orientation orientation;      ///< Quaternion orientation estimate.
    Vec3 angular_velocity;        ///< Angular velocity (rad/s).
    Vec3 linear_acceleration;     ///< Linear acceleration (m/s^2).
};

}  // namespace conduit
