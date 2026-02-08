

# File shm\_region.hpp

[**File List**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**internal**](dir_ee09fc54beaf3ba0ab27276c1e0adb4c.md) **>** [**shm\_region.hpp**](shm__region_8hpp.md)

[Go to the documentation of this file](shm__region_8hpp.md)


```C++
#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <string>

namespace conduit {
namespace internal {

class ShmRegion {
public:
    static ShmRegion create(const std::string& name, size_t size);

    static ShmRegion open(const std::string& name);

    static bool exists(const std::string& name);

    static bool wait_until_exists(const std::string& name,
                                  const std::atomic<bool>& running,
                                  std::chrono::milliseconds poll_interval = std::chrono::milliseconds(100));

    static void unlink(const std::string& name);

    ShmRegion(ShmRegion&& other) noexcept;
    ShmRegion& operator=(ShmRegion&& other) noexcept;
    ShmRegion(const ShmRegion&) = delete;
    ShmRegion& operator=(const ShmRegion&) = delete;

    ~ShmRegion();

    void* data() { return data_; }

    const void* data() const { return data_; }

    size_t size() const { return size_; }

    const std::string& name() const { return name_; }

private:
    ShmRegion(const std::string& name, void* data, size_t size);

    std::string name_;
    void* data_;
    size_t size_;
};

}  // namespace internal
}  // namespace conduit
```


