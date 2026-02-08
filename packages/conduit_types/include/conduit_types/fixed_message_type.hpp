#pragma once

/// @file fixed_message_type.hpp
/// @brief Base class and compile-time validation for fixed-size messages.
///
/// Rules for fixed message types:
///   1. Derive from FixedMessageType
///   2. Must be trivially copyable (no pointers, no std::string, no virtuals)
///   3. Must be standard layout
///   4. Published via memcpy — no serialization needed
///
/// @see validate_fixed_message_type, VariableMessageType

#include <cstddef>
#include <type_traits>

namespace conduit {

/// @brief Base class for fixed-size, trivially copyable message types.
///
/// Derive from this to create messages that are transmitted via zero-copy
/// memcpy. The type must be trivially copyable and standard layout.
///
/// @see validate_fixed_message_type
struct FixedMessageType {
protected:
    FixedMessageType() = default;
};

/// @brief Compile-time validation that T is a valid fixed message type.
///
/// Checks that T derives from FixedMessageType, is trivially copyable,
/// is standard layout, and is non-empty.
///
/// @tparam T The type to validate.
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
