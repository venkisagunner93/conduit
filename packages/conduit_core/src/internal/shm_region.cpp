#include "conduit_core/internal/shm_region.hpp"
#include "conduit_core/exceptions.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <thread>

namespace conduit {
namespace internal {

namespace {

std::string make_shm_path(const std::string& name) {
    return "/conduit_" + name;
}

}  // namespace

ShmRegion ShmRegion::create(const std::string& name, size_t size) {
    std::string path = make_shm_path(name);

    // Create shared memory (O_EXCL ensures it doesn't already exist)
    int fd = shm_open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            throw ShmError("Shared memory already exists: " + name);
        }
        throw ShmError("shm_open failed for '" + name + "': " + strerror(errno));
    }

    // Set size
    if (ftruncate(fd, static_cast<off_t>(size)) < 0) {
        int err = errno;
        close(fd);
        shm_unlink(path.c_str());
        throw ShmError("ftruncate failed for '" + name + "': " + strerror(err));
    }

    // Map memory
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);  // fd not needed after mmap

    if (ptr == MAP_FAILED) {
        int err = errno;
        shm_unlink(path.c_str());
        throw ShmError("mmap failed for '" + name + "': " + strerror(err));
    }

    // Zero initialize
    std::memset(ptr, 0, size);

    return ShmRegion(name, ptr, size);
}

ShmRegion ShmRegion::open(const std::string& name) {
    std::string path = make_shm_path(name);

    // Open existing shared memory
    int fd = shm_open(path.c_str(), O_RDWR, 0666);
    if (fd < 0) {
        throw ShmError("Shared memory not found: " + name);
    }

    // Get size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        int err = errno;
        close(fd);
        throw ShmError("fstat failed for '" + name + "': " + strerror(err));
    }

    size_t size = static_cast<size_t>(st.st_size);

    // Map memory
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (ptr == MAP_FAILED) {
        throw ShmError("mmap failed for '" + name + "': " + strerror(errno));
    }

    return ShmRegion(name, ptr, size);
}

bool ShmRegion::exists(const std::string& name) {
    std::string path = make_shm_path(name);

    int fd = shm_open(path.c_str(), O_RDONLY, 0);
    if (fd < 0) {
        return false;
    }

    close(fd);
    return true;
}

bool ShmRegion::wait_until_exists(const std::string& name,
                                  const std::atomic<bool>& running,
                                  std::chrono::milliseconds poll_interval) {
    while (running.load(std::memory_order_acquire)) {
        if (exists(name)) {
            return true;
        }
        std::this_thread::sleep_for(poll_interval);
    }
    return false;
}

void ShmRegion::unlink(const std::string& name) {
    std::string path = make_shm_path(name);
    shm_unlink(path.c_str());  // ignore errors
}

ShmRegion::ShmRegion(const std::string& name, void* data, size_t size)
    : name_(name), data_(data), size_(size) {}

ShmRegion::ShmRegion(ShmRegion&& other) noexcept
    : name_(std::move(other.name_)),
      data_(other.data_),
      size_(other.size_) {
    other.data_ = nullptr;
    other.size_ = 0;
}

ShmRegion& ShmRegion::operator=(ShmRegion&& other) noexcept {
    if (this != &other) {
        // Clean up existing
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

ShmRegion::~ShmRegion() {
    if (data_ != nullptr) {
        munmap(data_, size_);
        data_ = nullptr;
    }
}

}  // namespace internal
}  // namespace conduit
