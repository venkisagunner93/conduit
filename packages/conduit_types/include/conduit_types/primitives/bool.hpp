#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

struct Bool : public FixedMessageType {
    bool value;
};

}  // namespace conduit
