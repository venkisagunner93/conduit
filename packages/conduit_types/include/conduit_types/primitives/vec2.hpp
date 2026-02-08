#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

/// @brief Fixed-size 2D vector message type.
struct Vec2 : FixedMessageType {
    double x;  ///< X component.
    double y;  ///< Y component.
};

}  // namespace conduit
