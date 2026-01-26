#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <string>

namespace conduit {
namespace internal {

class ShmRegion {
public:
    // Create new shared memory region (publisher)
    static ShmRegion create(const std::string& name, size_t size);

    // Open existing shared memory region (subscriber)
    static ShmRegion open(const std::string& name);

    // Check if region exists
    static bool exists(const std::string& name);

    // Wait until region exists (for subscribers starting before publishers)
    // Returns true if region exists, false if stopped early (running became false)
    static bool wait_until_exists(const std::string& name,
                                  const std::atomic<bool>& running,
                                  std::chrono::milliseconds poll_interval = std::chrono::milliseconds(100));

    // Remove shared memory region
    static void unlink(const std::string& name);

    // Move-only (no copy)
    ShmRegion(ShmRegion&& other) noexcept;
    ShmRegion& operator=(ShmRegion&& other) noexcept;
    ShmRegion(const ShmRegion&) = delete;
    ShmRegion& operator=(const ShmRegion&) = delete;

    // Destructor unmaps memory (does NOT unlink)
    ~ShmRegion();

    // Accessors
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
