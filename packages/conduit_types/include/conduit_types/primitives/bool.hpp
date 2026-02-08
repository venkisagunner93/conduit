#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

/// @brief Fixed-size boolean message type.
struct Bool : public FixedMessageType {
    bool value;  ///< Boolean value.
};

}  // namespace conduit
