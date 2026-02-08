#pragma once

#include "conduit_types/fixed_message_type.hpp"

#include <cstdint>

namespace conduit {

/// @brief Fixed-size timestamp message type.
struct Time : public FixedMessageType {
    uint64_t nanoseconds;  ///< Time in nanoseconds.
};

}  // namespace conduit
