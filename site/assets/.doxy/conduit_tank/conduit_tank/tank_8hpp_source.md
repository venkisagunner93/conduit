

# File tank.hpp

[**File List**](files.md) **>** [**conduit\_tank**](dir_54377b39698324c476f35b30c1643219.md) **>** [**include**](dir_d2ca71ae5994f61a5fce416b805b40c7.md) **>** [**conduit\_tank**](dir_81a44e27c6bd9edd39a953e81d05e4bb.md) **>** [**tank.hpp**](tank_8hpp.md)

[Go to the documentation of this file](tank_8hpp.md)


```C++
#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace conduit {

class Tank {
public:
    explicit Tank(const std::string& output_path);
    ~Tank();

    // No copy, no move
    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;
    Tank(Tank&&) = delete;
    Tank& operator=(Tank&&) = delete;

    void add_topic(const std::string& topic);

    void start();

    void stop();

    bool recording() const;

    uint64_t message_count() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace conduit
```


