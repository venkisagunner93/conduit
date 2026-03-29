

# File variable\_message\_type.hpp

[**File List**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**variable\_message\_type.hpp**](variable__message__type_8hpp.md)

[Go to the documentation of this file](variable__message__type_8hpp.md)


```C++
#pragma once


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
```


