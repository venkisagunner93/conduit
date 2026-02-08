#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

struct Vec3 : FixedMessageType {
    double x;
    double y;
    double z;
};

}  // namespace conduit
