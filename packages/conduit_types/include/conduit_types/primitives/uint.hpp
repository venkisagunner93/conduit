#pragma once

#include "conduit_types/fixed_message_type.hpp"

#include <cstdint>

namespace conduit {

struct Uint : public FixedMessageType {
    uint64_t value;
};

}  // namespace conduit
