#pragma once

#include "conduit_types/fixed_message_type.hpp"

#include <cstdint>

namespace conduit {

struct Int : public FixedMessageType {
    int64_t value;
};

}  // namespace conduit
