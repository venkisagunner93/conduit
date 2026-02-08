#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "conduit_core/internal/ring_buffer.hpp"
#include "conduit_core/internal/shm_region.hpp"

namespace conduit {

struct PublisherOptions {
    uint32_t depth = 16;               // number of slots (power of 2)
    uint32_t max_message_size = 4096;  // max payload size in bytes
};

namespace internal {

class Publisher {
public:
    Publisher(const std::string& topic, const PublisherOptions& options = {});

    Publisher(Publisher&&) noexcept;
    Publisher& operator=(Publisher&&) noexcept;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    ~Publisher();

    bool publish(const void* data, size_t size);

    const std::string& topic() const { return topic_; }
    uint32_t max_message_size() const { return max_message_size_; }

private:
    std::string topic_;
    uint32_t max_message_size_;
    ShmRegion shm_;
    std::unique_ptr<RingBufferWriter> writer_;
};

}  // namespace internal

}  // namespace conduit

// Include message type traits for Publisher<T>
#include <conduit_types/fixed_message_type.hpp>
#include <conduit_types/variable_message_type.hpp>

namespace conduit {

template <typename T>
class Publisher {
public:
    Publisher(const std::string& topic, const PublisherOptions& options = {})
        : impl_(topic, options) {
        validate();
    }

    // Move-only
    Publisher(Publisher&&) noexcept = default;
    Publisher& operator=(Publisher&&) noexcept = default;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    bool publish(const T& msg) {
        if constexpr (std::is_base_of_v<FixedMessageType, T>) {
            return impl_.publish(&msg, sizeof(T));
        } else {
            size_t size = msg.serialized_size();
            buffer_.resize(size);
            msg.serialize(buffer_.data());
            return impl_.publish(buffer_.data(), size);
        }
    }

    const std::string& topic() const { return impl_.topic(); }
    uint32_t max_message_size() const { return impl_.max_message_size(); }

private:
    internal::Publisher impl_;
    std::vector<uint8_t> buffer_;  // serialization scratch space for variable types

    static constexpr void validate() {
        if constexpr (std::is_base_of_v<FixedMessageType, T>) {
            validate_fixed_message_type<T>();
        } else {
            validate_variable_message_type<T>();
        }
    }
};

}  // namespace conduit
