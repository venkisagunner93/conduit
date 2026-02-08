#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include "conduit_core/internal/ring_buffer.hpp"
#include "conduit_core/internal/shm_region.hpp"

namespace conduit {

/// @brief Configuration for topic subscriber.
struct SubscriberOptions {
    // reserved for future use
};

/// @brief Raw message received from a topic.
///
/// Contains a pointer into shared memory that is only valid until the
/// next call to take()/wait()/wait_for() on the same subscriber.
struct Message {
    const void* data;        ///< Pointer to payload in shared memory (transient).
    size_t size;             ///< Payload size in bytes.
    uint64_t sequence;       ///< Monotonically increasing message sequence number.
    uint64_t timestamp_ns;   ///< CLOCK_MONOTONIC_RAW timestamp in nanoseconds.
};

namespace internal {

/// @brief Low-level subscriber that reads raw bytes from a shared memory ring buffer.
///
/// Opens an existing shared memory region and claims a reader slot in the
/// lock-free SPMC ring buffer. Use the typed Subscriber<T> wrapper for
/// automatic deserialization.
///
/// @see conduit::Subscriber
class Subscriber {
public:
    /// @brief Construct a subscriber for the given topic.
    /// @param topic Topic name of the shared memory region to open.
    /// @param options Subscriber configuration (reserved for future use).
    /// @throws SubscriberError If shared memory cannot be opened or no reader slots available.
    Subscriber(const std::string& topic, const SubscriberOptions& options = {});

    /// @brief Move constructor.
    Subscriber(Subscriber&&) noexcept;
    /// @brief Move assignment operator.
    Subscriber& operator=(Subscriber&&) noexcept;
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    ~Subscriber();

    /// @brief Non-blocking read of the next message.
    /// @return The next message, or std::nullopt if no new message is available.
    std::optional<Message> take();

    /// @brief Block until a message is available.
    ///
    /// Uses futex-based signaling for zero CPU usage while idle.
    ///
    /// @return The next message.
    Message wait();

    /// @brief Block until a message is available or timeout expires.
    /// @param timeout Maximum time to wait.
    /// @return The next message, or std::nullopt on timeout.
    std::optional<Message> wait_for(std::chrono::nanoseconds timeout);

    /// @brief Get the topic name.
    /// @return Reference to the topic string.
    const std::string& topic() const { return topic_; }

private:
    std::string topic_;
    ShmRegion shm_;
    std::unique_ptr<RingBufferReader> reader_;
    int slot_;
};

}  // namespace internal

}  // namespace conduit

// Include message type traits for Subscriber<T>
#include <conduit_types/fixed_message_type.hpp>
#include <conduit_types/variable_message_type.hpp>

namespace conduit {

/// @brief Typed message with deserialized payload.
/// @tparam T The message type.
template <typename T>
struct TypedMessage {
    T data;                  ///< Deserialized message payload.
    uint64_t sequence;       ///< Monotonically increasing message sequence number.
    uint64_t timestamp_ns;   ///< CLOCK_MONOTONIC_RAW timestamp in nanoseconds.
};

/// @brief Type-safe subscriber that deserializes messages of type T.
///
/// For FixedMessageType derivatives, messages are deserialized via memcpy.
/// For VariableMessageType derivatives, T::deserialize() is called.
///
/// @tparam T Message type (must derive from FixedMessageType or VariableMessageType).
/// @see SubscriberOptions, Node::subscribe
template <typename T>
class Subscriber {
public:
    /// @brief Construct a typed subscriber for the given topic.
    /// @param topic Topic name of the shared memory region to open.
    /// @param options Subscriber configuration.
    Subscriber(const std::string& topic, const SubscriberOptions& options = {})
        : impl_(topic, options) {
        validate();
    }

    /// @brief Move constructor.
    Subscriber(Subscriber&&) noexcept = default;
    /// @brief Move assignment operator.
    Subscriber& operator=(Subscriber&&) noexcept = default;
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    /// @brief Non-blocking read of the next typed message.
    /// @return Deserialized message, or std::nullopt if no new message is available.
    std::optional<TypedMessage<T>> take() {
        auto msg = impl_.take();
        if (!msg) return std::nullopt;
        return convert(*msg);
    }

    /// @brief Block until a typed message is available.
    /// @return The next deserialized message.
    TypedMessage<T> wait() {
        return convert(impl_.wait());
    }

    /// @brief Block until a typed message is available or timeout expires.
    /// @param timeout Maximum time to wait.
    /// @return Deserialized message, or std::nullopt on timeout.
    std::optional<TypedMessage<T>> wait_for(std::chrono::nanoseconds timeout) {
        auto msg = impl_.wait_for(timeout);
        if (!msg) return std::nullopt;
        return convert(*msg);
    }

    /// @brief Get the topic name.
    /// @return Reference to the topic string.
    const std::string& topic() const { return impl_.topic(); }

private:
    internal::Subscriber impl_;

    static TypedMessage<T> convert(const Message& msg) {
        T data = [&]() {
            if constexpr (std::is_base_of_v<FixedMessageType, T>) {
                T d;
                std::memcpy(&d, msg.data, sizeof(T));
                return d;
            } else {
                return T::deserialize(
                    static_cast<const uint8_t*>(msg.data), msg.size);
            }
        }();
        return TypedMessage<T>{std::move(data), msg.sequence, msg.timestamp_ns};
    }

    static constexpr void validate() {
        if constexpr (std::is_base_of_v<FixedMessageType, T>) {
            validate_fixed_message_type<T>();
        } else {
            validate_variable_message_type<T>();
        }
    }
};

}  // namespace conduit
