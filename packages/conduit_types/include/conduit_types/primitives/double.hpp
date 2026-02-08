#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

/// @brief Fixed-size double-precision floating point message type.
struct Double : public FixedMessageType {
    double value;  ///< Double-precision value.
};

}  // namespace conduit
