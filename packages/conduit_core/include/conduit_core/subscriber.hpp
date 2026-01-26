#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "conduit_core/internal/ring_buffer.hpp"
#include "conduit_core/internal/shm_region.hpp"

namespace conduit {

struct SubscriberOptions {
    // reserved for future use
};

struct Message {
    const void* data;
    size_t size;
    uint64_t sequence;
    uint64_t timestamp_ns;
};

class Subscriber {
public:
    // Subscribe to topic
    Subscriber(const std::string& topic, const SubscriberOptions& options = {});

    // Move-only
    Subscriber(Subscriber&&) noexcept;
    Subscriber& operator=(Subscriber&&) noexcept;
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    ~Subscriber();

    // Non-blocking read
    std::optional<Message> take();

    // Blocking read
    Message wait();

    // Blocking read with timeout
    // Returns: message if received, nullopt if timeout
    std::optional<Message> wait_for(std::chrono::nanoseconds timeout);

    // Accessors
    const std::string& topic() const { return topic_; }

private:
    std::string topic_;
    internal::ShmRegion shm_;
    std::unique_ptr<internal::RingBufferReader> reader_;
    int slot_;
};

}  // namespace conduit
