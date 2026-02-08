#pragma once

#include "conduit_types/derived/orientation.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec2.hpp"

namespace conduit {

/// @brief 2D pose with position and orientation.
struct Pose2D : FixedMessageType {
    Header header;            ///< Timestamp and coordinate frame.
    Vec2 position;            ///< 2D position (x, y).
    Orientation orientation;  ///< Quaternion orientation (typically yaw-only for 2D).
};

}  // namespace conduit
