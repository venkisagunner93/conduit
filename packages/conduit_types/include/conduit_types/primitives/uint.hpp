#pragma once

#include "conduit_types/fixed_message_type.hpp"

#include <cstdint>

namespace conduit {

/// @brief Fixed-size unsigned 64-bit integer message type.
struct Uint : public FixedMessageType {
    uint64_t value;  ///< Unsigned integer value.
};

}  // namespace conduit
