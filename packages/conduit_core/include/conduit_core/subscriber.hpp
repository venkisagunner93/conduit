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

struct SubscriberOptions {
    // reserved for future use
};

struct Message {
    const void* data;
    size_t size;
    uint64_t sequence;
    uint64_t timestamp_ns;
};

namespace internal {

class Subscriber {
public:
    Subscriber(const std::string& topic, const SubscriberOptions& options = {});

    Subscriber(Subscriber&&) noexcept;
    Subscriber& operator=(Subscriber&&) noexcept;
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    ~Subscriber();

    std::optional<Message> take();
    Message wait();
    std::optional<Message> wait_for(std::chrono::nanoseconds timeout);

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

template <typename T>
struct TypedMessage {
    T data;
    uint64_t sequence;
    uint64_t timestamp_ns;
};

template <typename T>
class Subscriber {
public:
    Subscriber(const std::string& topic, const SubscriberOptions& options = {})
        : impl_(topic, options) {
        validate();
    }

    // Move-only
    Subscriber(Subscriber&&) noexcept = default;
    Subscriber& operator=(Subscriber&&) noexcept = default;
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    std::optional<TypedMessage<T>> take() {
        auto msg = impl_.take();
        if (!msg) return std::nullopt;
        return convert(*msg);
    }

    TypedMessage<T> wait() {
        return convert(impl_.wait());
    }

    std::optional<TypedMessage<T>> wait_for(std::chrono::nanoseconds timeout) {
        auto msg = impl_.wait_for(timeout);
        if (!msg) return std::nullopt;
        return convert(*msg);
    }

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
