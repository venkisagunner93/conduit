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

/// @brief Configuration for topic publisher and ring buffer sizing.
struct PublisherOptions {
    /// Number of message slots in the ring buffer (must be power of 2).
    uint32_t depth = 16;
    /// Maximum payload size in bytes. Messages exceeding this are rejected.
    uint32_t max_message_size = 4096;
};

namespace internal {

/// @brief Low-level publisher that writes raw bytes to a shared memory ring buffer.
///
/// Creates a shared memory region at `/dev/shm/conduit_{topic}` and manages
/// a lock-free SPMC ring buffer for zero-copy message delivery. Use the typed
/// Publisher<T> wrapper for type-safe publishing.
///
/// @see conduit::Publisher
class Publisher {
public:
    /// @brief Construct a publisher for the given topic.
    /// @param topic Topic name used to create the shared memory region.
    /// @param options Ring buffer configuration (depth and max message size).
    /// @throws PublisherError If shared memory creation fails.
    Publisher(const std::string& topic, const PublisherOptions& options = {});

    /// @brief Move constructor.
    Publisher(Publisher&&) noexcept;
    /// @brief Move assignment operator.
    Publisher& operator=(Publisher&&) noexcept;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    ~Publisher();

    /// @brief Publish raw data to the topic.
    /// @param data Pointer to the payload bytes.
    /// @param size Size of the payload in bytes.
    /// @return true if the message was written, false if size exceeds max_message_size.
    bool publish(const void* data, size_t size);

    /// @brief Get the topic name.
    /// @return Reference to the topic string.
    const std::string& topic() const { return topic_; }

    /// @brief Get the maximum allowed message size.
    /// @return Maximum payload size in bytes.
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

/// @brief Type-safe publisher that serializes messages of type T.
///
/// For FixedMessageType derivatives, messages are published via memcpy.
/// For VariableMessageType derivatives, serialize() is called into an
/// internal buffer before publishing.
///
/// @tparam T Message type (must derive from FixedMessageType or VariableMessageType).
/// @see PublisherOptions, Node::advertise
template <typename T>
class Publisher {
public:
    /// @brief Construct a typed publisher for the given topic.
    /// @param topic Topic name used to create the shared memory region.
    /// @param options Ring buffer configuration.
    Publisher(const std::string& topic, const PublisherOptions& options = {})
        : impl_(topic, options) {
        validate();
    }

    /// @brief Move constructor.
    Publisher(Publisher&&) noexcept = default;
    /// @brief Move assignment operator.
    Publisher& operator=(Publisher&&) noexcept = default;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    /// @brief Publish a typed message.
    ///
    /// Fixed types are published via memcpy. Variable types are serialized
    /// into an internal buffer first.
    ///
    /// @param msg The message to publish.
    /// @return true if the message was written, false if it exceeds max_message_size.
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

    /// @brief Get the topic name.
    /// @return Reference to the topic string.
    const std::string& topic() const { return impl_.topic(); }

    /// @brief Get the maximum allowed message size.
    /// @return Maximum payload size in bytes.
    uint32_t max_message_size() const { return impl_.max_message_size(); }

private:
    internal::Publisher impl_;
    std::vector<uint8_t> buffer_;  ///< Serialization scratch space for variable types.

    static constexpr void validate() {
        if constexpr (std::is_base_of_v<FixedMessageType, T>) {
            validate_fixed_message_type<T>();
        } else {
            validate_variable_message_type<T>();
        }
    }
};

}  // namespace conduit
