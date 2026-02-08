#pragma once

// VariableMessageType — base for variable-size messages that need serialization.
//
// Rules:
//   1. Derive from VariableMessageType
//   2. Implement serialized_size() and serialize(uint8_t*)
//   3. Provide a static deserialize(const uint8_t*, size_t) -> T method
//   4. Must NOT be trivially copyable (enforced as sanity check)
//
// Validation:
//   Call validate_variable_message_type<T>() to check at compile time.

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace conduit {

class VariableMessageType {
public:
    virtual ~VariableMessageType() = default;

    virtual size_t serialized_size() const = 0;
    virtual void serialize(uint8_t* buffer) const = 0;

protected:
    VariableMessageType() = default;
};

namespace detail {

template <typename T, typename = void>
struct has_deserialize : std::false_type {};

template <typename T>
struct has_deserialize<T,
    std::void_t<decltype(T::deserialize(std::declval<const uint8_t*>(), std::declval<size_t>()))>>
    : std::is_same<T, decltype(T::deserialize(std::declval<const uint8_t*>(), std::declval<size_t>()))> {};

}  // namespace detail

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
