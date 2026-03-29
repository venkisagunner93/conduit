

# File fixed\_message\_type.hpp

[**File List**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**fixed\_message\_type.hpp**](fixed__message__type_8hpp.md)

[Go to the documentation of this file](fixed__message__type_8hpp.md)


```C++
#pragma once


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
```


