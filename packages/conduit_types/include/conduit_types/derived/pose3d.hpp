#pragma once

#include "conduit_types/derived/orientation.hpp"
#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/header.hpp"
#include "conduit_types/primitives/vec3.hpp"

namespace conduit {

struct Pose3D : FixedMessageType {
    Header header;
    Vec3 position;
    Orientation orientation;
};

}  // namespace conduit
