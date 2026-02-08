#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

struct Double : public FixedMessageType {
    double value;
};

}  // namespace conduit
