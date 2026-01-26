#include "conduit_core/subscriber.hpp"
#include "conduit_core/exceptions.hpp"

namespace conduit {

Subscriber::Subscriber(const std::string& topic, const SubscriberOptions& options)
    : topic_(topic),
      shm_(internal::ShmRegion::open(topic)),
      reader_(std::make_unique<internal::RingBufferReader>(shm_.data(), shm_.size())),
      slot_(reader_->claim_slot()) {
    (void)options;  // reserved for future use

    if (slot_ < 0) {
        throw SubscriberError("Too many subscribers for topic: " + topic);
    }
}

Subscriber::Subscriber(Subscriber&& other) noexcept
    : topic_(std::move(other.topic_)),
      shm_(std::move(other.shm_)),
      reader_(std::move(other.reader_)),
      slot_(other.slot_) {
    other.slot_ = -1;
}

Subscriber& Subscriber::operator=(Subscriber&& other) noexcept {
    if (this != &other) {
        // Release current slot
        if (slot_ >= 0 && reader_) {
            reader_->release_slot(slot_);
        }

        topic_ = std::move(other.topic_);
        shm_ = std::move(other.shm_);
        reader_ = std::move(other.reader_);
        slot_ = other.slot_;

        other.slot_ = -1;
    }
    return *this;
}

Subscriber::~Subscriber() {
    if (slot_ >= 0 && reader_) {
        reader_->release_slot(slot_);
    }
}

std::optional<Message> Subscriber::take() {
    auto result = reader_->try_read(slot_);
    if (!result) {
        return std::nullopt;
    }

    return Message{
        .data = result->data,
        .size = result->size,
        .sequence = result->sequence,
        .timestamp_ns = result->timestamp_ns
    };
}

Message Subscriber::wait() {
    auto result = reader_->wait(slot_);
    // wait() always returns a value (blocks until data available)
    return Message{
        .data = result->data,
        .size = result->size,
        .sequence = result->sequence,
        .timestamp_ns = result->timestamp_ns
    };
}

std::optional<Message> Subscriber::wait_for(std::chrono::nanoseconds timeout) {
    auto result = reader_->wait_for(slot_, timeout);
    if (!result) {
        return std::nullopt;
    }

    return Message{
        .data = result->data,
        .size = result->size,
        .sequence = result->sequence,
        .timestamp_ns = result->timestamp_ns
    };
}

}  // namespace conduit
