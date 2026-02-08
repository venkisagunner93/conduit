#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <string>

namespace conduit {
namespace internal {

/// @brief RAII wrapper for a POSIX shared memory region.
///
/// Provides factory methods to create (publisher) or open (subscriber)
/// a named shared memory region at `/dev/shm/conduit_{name}`. The region
/// is memory-mapped on construction and unmapped on destruction, but
/// the shared memory file is **not** unlinked automatically — call
/// unlink() explicitly when the topic is retired.
///
/// @see Publisher, Subscriber
class ShmRegion {
public:
    /// @brief Create a new shared memory region (used by publishers).
    /// @param name Region name (becomes `/dev/shm/conduit_{name}`).
    /// @param size Size in bytes.
    /// @return A mapped ShmRegion.
    /// @throws ShmError If shm_open or mmap fails.
    static ShmRegion create(const std::string& name, size_t size);

    /// @brief Open an existing shared memory region (used by subscribers).
    /// @param name Region name.
    /// @return A mapped ShmRegion.
    /// @throws ShmError If the region does not exist or mmap fails.
    static ShmRegion open(const std::string& name);

    /// @brief Check if a shared memory region exists.
    /// @param name Region name.
    /// @return true if the region exists.
    static bool exists(const std::string& name);

    /// @brief Poll until the shared memory region exists or running becomes false.
    /// @param name Region name.
    /// @param running Atomic flag checked each poll cycle; set to false to abort.
    /// @param poll_interval Time between existence checks.
    /// @return true if the region now exists, false if stopped early.
    static bool wait_until_exists(const std::string& name,
                                  const std::atomic<bool>& running,
                                  std::chrono::milliseconds poll_interval = std::chrono::milliseconds(100));

    /// @brief Remove the shared memory file from the filesystem.
    /// @param name Region name.
    static void unlink(const std::string& name);

    /// @brief Move constructor.
    ShmRegion(ShmRegion&& other) noexcept;
    /// @brief Move assignment operator.
    ShmRegion& operator=(ShmRegion&& other) noexcept;
    ShmRegion(const ShmRegion&) = delete;
    ShmRegion& operator=(const ShmRegion&) = delete;

    /// @brief Destructor. Unmaps the memory region (does **not** unlink).
    ~ShmRegion();

    /// @brief Get a writable pointer to the mapped memory.
    /// @return Pointer to the start of the region.
    void* data() { return data_; }

    /// @brief Get a read-only pointer to the mapped memory.
    /// @return Const pointer to the start of the region.
    const void* data() const { return data_; }

    /// @brief Get the size of the mapped region.
    /// @return Size in bytes.
    size_t size() const { return size_; }

    /// @brief Get the region name.
    /// @return Reference to the name string.
    const std::string& name() const { return name_; }

private:
    ShmRegion(const std::string& name, void* data, size_t size);

    std::string name_;
    void* data_;
    size_t size_;
};

}  // namespace internal
}  // namespace conduit
