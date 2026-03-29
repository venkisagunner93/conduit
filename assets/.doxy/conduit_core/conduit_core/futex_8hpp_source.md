

# File futex.hpp

[**File List**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**internal**](dir_ee09fc54beaf3ba0ab27276c1e0adb4c.md) **>** [**futex.hpp**](futex_8hpp.md)

[Go to the documentation of this file](futex_8hpp.md)


```C++
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>

namespace conduit::internal {

bool futex_wait(
    std::atomic<uint32_t>* futex_word,
    uint32_t expected_value,
    std::optional<std::chrono::nanoseconds> timeout = std::nullopt
);

int futex_wake(std::atomic<uint32_t>* futex_word, int count = 1);

int futex_wake_all(std::atomic<uint32_t>* futex_word);

}  // namespace conduit::internal
```


