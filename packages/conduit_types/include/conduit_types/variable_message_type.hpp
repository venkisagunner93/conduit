#pragma once

/// @file variable_message_type.hpp
/// @brief Base class and compile-time validation for variable-size messages.
///
/// Rules for variable message types:
///   1. Derive from VariableMessageType
///   2. Implement serialized_size() and serialize(uint8_t*)
///   3. Provide a static deserialize(const uint8_t*, size_t) -> T method
///   4. Must NOT be trivially copyable (enforced as sanity check)
///
/// @see validate_variable_message_type, FixedMessageType

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace conduit {

/// @brief Base class for variable-size messages requiring serialization.
///
/// Derive from this to create messages with dynamic-size fields (e.g. strings).
/// Subclasses must implement serialized_size(), serialize(), and a static
/// deserialize() factory method.
///
/// @see validate_variable_message_type, WriteBuffer, ReadBuffer
class VariableMessageType {
public:
    virtual ~VariableMessageType() = default;

    /// @brief Compute the serialized size in bytes.
    /// @return Number of bytes needed for serialize().
    virtual size_t serialized_size() const = 0;

    /// @brief Serialize the message into a pre-allocated buffer.
    /// @param buffer Output buffer (must have at least serialized_size() bytes).
    virtual void serialize(uint8_t* buffer) const = 0;

protected:
    VariableMessageType() = default;
};

namespace detail {

/// @cond INTERNAL
template <typename T, typename = void>
struct has_deserialize : std::false_type {};

template <typename T>
struct has_deserialize<T,
    std::void_t<decltype(T::deserialize(std::declval<const uint8_t*>(), std::declval<size_t>()))>>
    : std::is_same<T, decltype(T::deserialize(std::declval<const uint8_t*>(), std::declval<size_t>()))> {};
/// @endcond

}  // namespace detail

/// @brief Compile-time validation that T is a valid variable message type.
///
/// Checks that T derives from VariableMessageType, is not trivially copyable,
/// and provides a static `T deserialize(const uint8_t*, size_t)` method.
///
/// @tparam T The type to validate.
template <typename T>
constexpr void validate_variable_message_type() {
    static_assert(std::is_base_of_v<VariableMessageType, T>,
        "T must derive from VariableMessageType");
    static_assert(!std::is_trivially_copyable_v<T>,
        "T must not be trivially copyable (variable message types require serialization)");
    static_assert(detail::has_deserialize<T>::value,
        "T must provide static T deserialize(const uint8_t*, size_t)");
}

}  // namespace conduit
