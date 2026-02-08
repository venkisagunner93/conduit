#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

struct Vec2 : FixedMessageType {
    double x;
    double y;
};

}  // namespace conduit
