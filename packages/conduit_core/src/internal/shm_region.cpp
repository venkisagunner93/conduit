/**
 * @file shm_region.cpp
 * @brief Shared Memory Region - The foundation for zero-copy messaging
 *
 * == What is Shared Memory? ==
 *
 * Normally, when two programs want to share data, they copy it:
 *   Program A -> [copy to kernel] -> [copy to Program B]
 *
 * Shared memory eliminates these copies. Both programs see the SAME physical
 * memory, just mapped into their address spaces:
 *
 *   Program A's memory:  [....][SHARED][....]
 *                              |
 *                              v  (same physical RAM)
 *   Program B's memory:  [....][SHARED][....]
 *
 * On Linux, shared memory lives in /dev/shm/ (a RAM-based filesystem).
 * Conduit creates files like /dev/shm/conduit_imu for a topic named "imu".
 *
 * == How an int flows through shared memory ==
 *
 * Publisher:
 *   1. Creates /dev/shm/conduit_mytopic (ShmRegion::create)
 *   2. Gets a pointer to memory (ShmRegion::data())
 *   3. Writes: *(int*)ptr = 42
 *
 * Subscriber:
 *   1. Opens /dev/shm/conduit_mytopic (ShmRegion::open)
 *   2. Gets pointer to SAME memory (ShmRegion::data())
 *   3. Reads: int value = *(int*)ptr  // sees 42!
 *
 * No copying! The int exists in one place, visible to both programs.
 *
 * == How a PointCloud flows through ==
 *
 * Same concept, but more bytes:
 *
 * Publisher:
 *   struct PointCloud { float x[1000000], y[1000000], z[1000000]; };  // ~12MB
 *   memcpy(shm_ptr, &cloud, sizeof(cloud));  // Write to shared memory
 *
 * Subscriber:
 *   PointCloud* cloud = (PointCloud*)shm_ptr;  // Just cast the pointer!
 *   float first_x = cloud->x[0];  // Read directly, no copy
 *
 * A 12MB point cloud is never copied - both processes access the same RAM.
 */

#include "conduit_core/internal/shm_region.hpp"
#include "conduit_core/exceptions.hpp"

#include <sys/mman.h>   // mmap, munmap - memory mapping functions
#include <sys/stat.h>   // fstat - get file info
#include <fcntl.h>      // O_CREAT, O_RDWR - file open flags
#include <unistd.h>     // close, ftruncate - POSIX functions
#include <cerrno>       // errno - error codes
#include <cstring>      // strerror, memset
#include <thread>       // sleep_for

namespace conduit {
namespace internal {

namespace {

/**
 * Convert a topic name to a shared memory path.
 *
 * Example: "imu" -> "/conduit_imu"
 *
 * The leading "/" is required by POSIX shared memory (shm_open).
 * The actual file appears at /dev/shm/conduit_imu on Linux.
 */
std::string make_shm_path(const std::string& name) {
    return "/conduit_" + name;
}

}  // namespace

/**
 * Create a new shared memory region (used by Publisher).
 *
 * @param name  Topic name (e.g., "imu", "lidar", "camera")
 * @param size  Bytes to allocate (must fit ring buffer + all messages)
 *
 * Steps:
 * 1. shm_open() - Creates /dev/shm/conduit_{name} (like creating a file)
 * 2. ftruncate() - Sets the file size (allocates RAM)
 * 3. mmap() - Maps the file into our address space (gives us a pointer)
 * 4. memset() - Zeros the memory (clean slate)
 *
 * After this, we have a pointer to `size` bytes of shared memory.
 *
 * Error handling: If the topic already exists (another publisher), we fail.
 * This prevents multiple writers to the same topic (single-producer design).
 */
ShmRegion ShmRegion::create(const std::string& name, size_t size) {
    std::string path = make_shm_path(name);

    // Step 1: Create shared memory object
    // O_CREAT = create if doesn't exist
    // O_EXCL  = fail if already exists (ensures single publisher)
    // O_RDWR  = read and write access
    // 0666   = permissions (rw for everyone)
    int fd = shm_open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            // Another publisher already created this topic
            throw ShmError("Shared memory already exists: " + name);
        }
        throw ShmError("shm_open failed for '" + name + "': " + strerror(errno));
    }

    // Step 2: Set the size of shared memory
    // ftruncate sets file size - this allocates the actual RAM
    if (ftruncate(fd, static_cast<off_t>(size)) < 0) {
        int err = errno;
        close(fd);
        shm_unlink(path.c_str());  // Clean up the file we just created
        throw ShmError("ftruncate failed for '" + name + "': " + strerror(err));
    }

    // Step 3: Map shared memory into our address space
    // mmap returns a pointer we can use like regular memory
    // MAP_SHARED = changes visible to other processes (the key feature!)
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // File descriptor no longer needed after mmap - the mapping persists
    close(fd);

    if (ptr == MAP_FAILED) {
        int err = errno;
        shm_unlink(path.c_str());
        throw ShmError("mmap failed for '" + name + "': " + strerror(err));
    }

    // Step 4: Zero-initialize the memory
    // Important for ring buffer header initialization
    std::memset(ptr, 0, size);

    return ShmRegion(name, ptr, size);
}

/**
 * Open an existing shared memory region (used by Subscriber).
 *
 * @param name  Topic name to subscribe to
 *
 * Similar to create(), but:
 * - No O_CREAT/O_EXCL (topic must already exist)
 * - No ftruncate (size already set by publisher)
 * - No memset (don't want to overwrite publisher's data!)
 *
 * Returns a pointer to the SAME memory the publisher is using.
 */
ShmRegion ShmRegion::open(const std::string& name) {
    std::string path = make_shm_path(name);

    // Open existing shared memory (no O_CREAT)
    int fd = shm_open(path.c_str(), O_RDWR, 0666);
    if (fd < 0) {
        throw ShmError("Shared memory not found: " + name);
    }

    // Get size from file stats (publisher already set this)
    struct stat st;
    if (fstat(fd, &st) < 0) {
        int err = errno;
        close(fd);
        throw ShmError("fstat failed for '" + name + "': " + strerror(err));
    }

    size_t size = static_cast<size_t>(st.st_size);

    // Map into our address space
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (ptr == MAP_FAILED) {
        throw ShmError("mmap failed for '" + name + "': " + strerror(errno));
    }

    return ShmRegion(name, ptr, size);
}

/**
 * Check if a topic's shared memory exists.
 *
 * Used by subscribers to check if a publisher has started.
 * Also used by CLI tools (conduit topics, conduit info).
 */
bool ShmRegion::exists(const std::string& name) {
    std::string path = make_shm_path(name);

    // Try to open read-only (don't create)
    int fd = shm_open(path.c_str(), O_RDONLY, 0);
    if (fd < 0) {
        return false;  // Doesn't exist
    }

    close(fd);
    return true;
}

/**
 * Wait until a topic's shared memory exists.
 *
 * Subscribers use this when they start before the publisher.
 * Polls periodically until either:
 * - The topic appears (returns true)
 * - running becomes false (returns false, for clean shutdown)
 *
 * @param name           Topic to wait for
 * @param running        Atomic flag (set to false to stop waiting)
 * @param poll_interval  How often to check (default 100ms)
 */
bool ShmRegion::wait_until_exists(const std::string& name,
                                  const std::atomic<bool>& running,
                                  std::chrono::milliseconds poll_interval) {
    while (running.load(std::memory_order_acquire)) {
        if (exists(name)) {
            return true;  // Topic appeared!
        }
        std::this_thread::sleep_for(poll_interval);
    }
    return false;  // Stopped early (shutdown requested)
}

/**
 * Remove shared memory file.
 *
 * Called by publisher during cleanup. After unlink:
 * - New processes can't open it
 * - Existing mappings still work until they unmap
 * - Memory is freed when last process unmaps
 */
void ShmRegion::unlink(const std::string& name) {
    std::string path = make_shm_path(name);
    shm_unlink(path.c_str());  // Ignore errors (might not exist)
}

// === Constructor and move semantics ===

ShmRegion::ShmRegion(const std::string& name, void* data, size_t size)
    : name_(name), data_(data), size_(size) {}

// Move constructor - transfers ownership
ShmRegion::ShmRegion(ShmRegion&& other) noexcept
    : name_(std::move(other.name_)),
      data_(other.data_),
      size_(other.size_) {
    // Prevent other's destructor from unmapping
    other.data_ = nullptr;
    other.size_ = 0;
}

// Move assignment
ShmRegion& ShmRegion::operator=(ShmRegion&& other) noexcept {
    if (this != &other) {
        // Clean up our current mapping
        if (data_ != nullptr) {
            munmap(data_, size_);
        }

        name_ = std::move(other.name_);
        data_ = other.data_;
        size_ = other.size_;

        other.data_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

/**
 * Destructor unmaps memory.
 *
 * Note: Does NOT unlink! The shared memory file persists.
 * This is intentional - publisher must explicitly unlink when done.
 */
ShmRegion::~ShmRegion() {
    if (data_ != nullptr) {
        munmap(data_, size_);
        data_ = nullptr;
    }
}

}  // namespace internal
}  // namespace conduit
