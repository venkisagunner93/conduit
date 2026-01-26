#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "conduit_core/internal/ring_buffer.hpp"
#include "conduit_core/internal/shm_region.hpp"

namespace conduit {

struct PublisherOptions {
    uint32_t depth = 16;               // number of slots (power of 2)
    uint32_t max_message_size = 4096;  // max payload size in bytes
};

class Publisher {
public:
    // Create publisher for topic
    Publisher(const std::string& topic, const PublisherOptions& options = {});

    // Move-only
    Publisher(Publisher&&) noexcept;
    Publisher& operator=(Publisher&&) noexcept;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    ~Publisher();

    // Publish message
    // Returns: true if published, false if message too large
    bool publish(const void* data, size_t size);

    // Accessors
    const std::string& topic() const { return topic_; }
    uint32_t max_message_size() const { return max_message_size_; }

private:
    std::string topic_;
    uint32_t max_message_size_;
    internal::ShmRegion shm_;
    std::unique_ptr<internal::RingBufferWriter> writer_;
};

}  // namespace conduit
