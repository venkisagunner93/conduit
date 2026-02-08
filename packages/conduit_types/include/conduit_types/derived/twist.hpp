#pragma once

#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

struct Twist : FixedMessageType {
    Header header;
    Vec3 linear;
    Vec3 angular;
};

}  // namespace conduit
