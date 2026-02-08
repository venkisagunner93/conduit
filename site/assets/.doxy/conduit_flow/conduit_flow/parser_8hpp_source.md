

# File parser.hpp

[**File List**](files.md) **>** [**conduit\_flow**](dir_0d13c9fe7659a255034ddc49adfce1eb.md) **>** [**include**](dir_fb8309341c6f8659fa21828ed1417524.md) **>** [**conduit\_flow**](dir_288fb1ab69c9ae4a94f91c5befae3aa0.md) **>** [**parser.hpp**](parser_8hpp.md)

[Go to the documentation of this file](parser_8hpp.md)


```C++
#pragma once

#include "conduit_flow/flow.hpp"
#include <string>

namespace conduit::flow {

FlowConfig parse_file(const std::string& path);

FlowConfig parse_string(const std::string& yaml);

}  // namespace conduit::flow
```


