#pragma once

// FixedMessageType — base for fixed-size, trivially copyable messages.
//
// Rules:
//   1. Derive from FixedMessageType
//   2. Must be trivially copyable (no pointers, no std::string, no virtuals)
//   3. Must be standard layout
//   4. Published via memcpy — no serialization needed
//
// Validation:
//   Call validate_fixed_message_type<T>() to check at compile time.

#include <cstddef>
#include <type_traits>

namespace conduit {

struct FixedMessageType {
protected:
    FixedMessageType() = default;
};

template <typename T>
constexpr void validate_fixed_message_type() {
    static_assert(std::is_base_of_v<FixedMessageType, T>,
        "T must derive from FixedMessageType");
    static_assert(std::is_trivially_copyable_v<T>,
        "T must be trivially copyable (no pointers, strings, or virtual functions)");
    static_assert(std::is_standard_layout_v<T>,
        "T must be standard layout");
    static_assert(sizeof(T) > 0,
        "T must not be empty");
}

}  // namespace conduit
