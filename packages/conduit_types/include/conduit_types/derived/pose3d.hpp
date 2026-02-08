#pragma once

#include "conduit_types/derived/orientation.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

/// @brief 3D pose with position and orientation.
struct Pose3D : FixedMessageType {
    Header header;            ///< Timestamp and coordinate frame.
    Vec3 position;            ///< 3D position (x, y, z).
    Orientation orientation;  ///< Quaternion orientation.
};

}  // namespace conduit
