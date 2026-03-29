

# File executor.hpp

[**File List**](files.md) **>** [**conduit\_flow**](dir_0d13c9fe7659a255034ddc49adfce1eb.md) **>** [**include**](dir_fb8309341c6f8659fa21828ed1417524.md) **>** [**conduit\_flow**](dir_288fb1ab69c9ae4a94f91c5befae3aa0.md) **>** [**executor.hpp**](executor_8hpp.md)

[Go to the documentation of this file](executor_8hpp.md)


```C++
#pragma once

#include "conduit_flow/flow.hpp"
#include <memory>

namespace conduit::flow {

struct ExecutorOptions {
    bool verbose = true;  
};

class Executor {
public:
    explicit Executor(ExecutorOptions options = {});
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    int run(const FlowConfig& config);

    void shutdown();

    bool running() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace conduit::flow
```


