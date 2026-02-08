#pragma once

#include "conduit_types/derived/orientation.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

struct Imu : FixedMessageType {
    Header header;
    Orientation orientation;
    Vec3 angular_velocity;
    Vec3 linear_acceleration;
};

}  // namespace conduit
