#pragma once

#include "conduit_types/derived/pose3d.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

struct Odometry : FixedMessageType {
    Header header;
    char child_frame[64];
    Pose3D pose;
    Vec3 linear_velocity;
    Vec3 angular_velocity;
};

}  // namespace conduit
