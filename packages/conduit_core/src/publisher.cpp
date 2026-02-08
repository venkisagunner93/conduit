#include "conduit_core/publisher.hpp"
#include "conduit_core/exceptions.hpp"

namespace conduit {

internal::Publisher::Publisher(const std::string& topic, const PublisherOptions& options)
    : topic_(topic),
      max_message_size_(options.max_message_size),
      shm_(internal::ShmRegion::create(
          topic,
          internal::calculate_region_size({
              options.depth,
              static_cast<uint32_t>(internal::SLOT_HEADER_SIZE + options.max_message_size)
          })
      )),
      writer_(std::make_unique<internal::RingBufferWriter>(
          shm_.data(),
          shm_.size(),
          internal::RingBufferConfig{
              options.depth,
              static_cast<uint32_t>(internal::SLOT_HEADER_SIZE + options.max_message_size)
          }
      )) {
    writer_->initialize();
}

internal::Publisher::Publisher(Publisher&& other) noexcept
    : topic_(std::move(other.topic_)),
      max_message_size_(other.max_message_size_),
      shm_(std::move(other.shm_)),
      writer_(std::move(other.writer_)) {
    other.max_message_size_ = 0;
}

internal::Publisher& internal::Publisher::operator=(Publisher&& other) noexcept {
    if (this != &other) {
        // Clean up current state
        if (writer_) {
            internal::ShmRegion::unlink(topic_);
        }

        topic_ = std::move(other.topic_);
        max_message_size_ = other.max_message_size_;
        shm_ = std::move(other.shm_);
        writer_ = std::move(other.writer_);

        other.max_message_size_ = 0;
    }
    return *this;
}

internal::Publisher::~Publisher() {
    // Unlink shared memory so it's removed when publisher is destroyed
    if (writer_) {
        internal::ShmRegion::unlink(topic_);
    }
}

bool internal::Publisher::publish(const void* data, size_t size) {
    return writer_->try_write(data, size);
}

}  // namespace conduit
