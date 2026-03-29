

# File header.hpp

[**File List**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**header.hpp**](header_8hpp.md)

[Go to the documentation of this file](header_8hpp.md)


```C++
#pragma once

#include <cstdint>
#include <cstring>

namespace conduit {

struct Header {
    uint64_t timestamp_ns;  
    char frame[64];         
};

inline void set_frame(char (&dst)[64], const char* src) {
    std::strncpy(dst, src, 63);
    dst[63] = '\0';
}

}  // namespace conduit
```


