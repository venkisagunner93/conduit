#pragma once

#include "conduit_types/fixed_message_type.hpp"

#include <cstdint>

namespace conduit {

struct Time : public FixedMessageType {
    uint64_t nanoseconds;
};

}  // namespace conduit
