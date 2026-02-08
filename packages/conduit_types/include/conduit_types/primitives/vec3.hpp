#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

/// @brief Fixed-size 3D vector message type.
struct Vec3 : FixedMessageType {
    double x;  ///< X component.
    double y;  ///< Y component.
    double z;  ///< Z component.
};

}  // namespace conduit
