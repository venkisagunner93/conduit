

# File exceptions.hpp

[**File List**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**exceptions.hpp**](exceptions_8hpp.md)

[Go to the documentation of this file](exceptions_8hpp.md)


```C++
#pragma once

#include <stdexcept>
#include <string>

namespace conduit {

class ConduitError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ShmError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class RingBufferError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class PublisherError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class SubscriberError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class NodeError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class TankError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

}  // namespace conduit
```


