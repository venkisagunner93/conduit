#pragma once

#include "conduit_types/fixed_message_type.hpp"

#include <cstdint>

namespace conduit {

/// @brief Fixed-size signed 64-bit integer message type.
struct Int : public FixedMessageType {
    int64_t value;  ///< Signed integer value.
};

}  // namespace conduit
