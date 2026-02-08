#pragma once

#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

/// @brief Linear and angular velocity in 3D.
struct Twist : FixedMessageType {
    Header header;   ///< Timestamp and coordinate frame.
    Vec3 linear;     ///< Linear velocity (m/s) in x, y, z.
    Vec3 angular;    ///< Angular velocity (rad/s) in x, y, z.
};

}  // namespace conduit
