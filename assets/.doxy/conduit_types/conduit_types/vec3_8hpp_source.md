

# File vec3.hpp

[**File List**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**primitives**](dir_5abad28eb9f480620bca24dc64f0d7e4.md) **>** [**vec3.hpp**](vec3_8hpp.md)

[Go to the documentation of this file](vec3_8hpp.md)


```C++
#pragma once

#include "conduit_types/fixed_message_type.hpp"

namespace conduit {

struct Vec3 : FixedMessageType {
    double x;  
    double y;  
    double z;  
};

}  // namespace conduit
```


