#include "conduit_core/internal/ring_buffer.hpp"
#include "conduit_core/internal/futex.hpp"
#include "conduit_core/internal/time.hpp"

#include <cassert>
#include <cstring>

namespace conduit {
namespace internal {

// RingBufferWriter

RingBufferWriter::RingBufferWriter(void* region, size_t region_size, const RingBufferConfig& config)
    : header_(static_cast<RingBufferHeader*>(region)),
      slots_(static_cast<uint8_t*>(region) + sizeof(RingBufferHeader)),
      slot_size_(config.slot_size),
      slot_count_(config.slot_count),
      slot_count_mask_(config.slot_count - 1) {
    assert(is_power_of_two(config.slot_count));
    assert(region_size >= calculate_region_size(config));
}

void RingBufferWriter::initialize() {
    header_->slot_count = slot_count_;
    header_->slot_size = slot_size_;
    header_->max_subscribers = MAX_SUBSCRIBERS;
    header_->padding0 = 0;
    header_->write_idx.store(0, std::memory_order_relaxed);
    header_->subscriber_mask.store(0, std::memory_order_relaxed);
    header_->futex_word.store(0, std::memory_order_relaxed);

    for (size_t i = 0; i < MAX_SUBSCRIBERS; ++i) {
        header_->read_idx[i].value.store(0, std::memory_order_relaxed);
    }

    std::atomic_thread_fence(std::memory_order_release);
}

bool RingBufferWriter::try_write(const void* data, size_t len) {
    // Check message fits
    if (len + SLOT_HEADER_SIZE > slot_size_) {
        return false;
    }

    // Get current write position
    uint64_t idx = header_->write_idx.load(std::memory_order_relaxed);

    // Calculate slot pointer
    uint32_t slot_idx = static_cast<uint32_t>(idx & slot_count_mask_);
    uint8_t* slot_ptr = slots_ + (static_cast<size_t>(slot_idx) * slot_size_);

    // Get timestamp
    uint64_t timestamp_ns = get_timestamp_ns();

    // Write slot header: size(4) + sequence(8) + timestamp(8) = 20 bytes
    uint32_t size32 = static_cast<uint32_t>(len);
    std::memcpy(slot_ptr + 0, &size32, sizeof(uint32_t));
    std::memcpy(slot_ptr + 4, &idx, sizeof(uint64_t));
    std::memcpy(slot_ptr + 12, &timestamp_ns, sizeof(uint64_t));

    // Write payload
    std::memcpy(slot_ptr + SLOT_HEADER_SIZE, data, len);

    // Publish (release ensures writes visible before index update)
    header_->write_idx.store(idx + 1, std::memory_order_release);

    // Signal waiters
    header_->futex_word.fetch_add(1, std::memory_order_release);
    futex_wake_all(&header_->futex_word);

    return true;
}

// RingBufferReader

RingBufferReader::RingBufferReader(void* region, size_t region_size)
    : header_(static_cast<RingBufferHeader*>(region)),
      slots_(static_cast<uint8_t*>(region) + sizeof(RingBufferHeader)),
      slot_size_(header_->slot_size),
      slot_count_mask_(header_->slot_count - 1) {
    (void)region_size;  // Used for assertions in debug builds if needed
}

int RingBufferReader::claim_slot() {
    while (true) {
        uint32_t mask = header_->subscriber_mask.load(std::memory_order_acquire);

        // Find first free bit
        for (uint32_t i = 0; i < header_->max_subscribers; ++i) {
            if (!(mask & (1u << i))) {
                // Attempt to claim
                uint32_t new_mask = mask | (1u << i);
                if (header_->subscriber_mask.compare_exchange_weak(
                        mask, new_mask,
                        std::memory_order_acq_rel,
                        std::memory_order_acquire)) {
                    // Initialize read index to current write position
                    uint64_t write_idx = header_->write_idx.load(std::memory_order_acquire);
                    header_->read_idx[i].value.store(write_idx, std::memory_order_release);
                    return static_cast<int>(i);
                }
                // CAS failed, break and retry outer loop
                break;
            }
        }

        // All slots taken?
        if (mask == (1u << header_->max_subscribers) - 1) {
            return -1;
        }
    }
}

void RingBufferReader::release_slot(int slot) {
    uint32_t bit = 1u << static_cast<uint32_t>(slot);
    header_->subscriber_mask.fetch_and(~bit, std::memory_order_release);
}

std::optional<ReadResult> RingBufferReader::try_read(int slot) {
    uint64_t read_idx = header_->read_idx[slot].value.load(std::memory_order_relaxed);
    uint64_t write_idx = header_->write_idx.load(std::memory_order_acquire);

    // Empty?
    if (read_idx >= write_idx) {
        return std::nullopt;
    }

    // Fallen behind? (data overwritten)
    if (write_idx - read_idx > header_->slot_count) {
        // Skip to oldest available
        read_idx = write_idx - header_->slot_count;
        header_->read_idx[slot].value.store(read_idx, std::memory_order_relaxed);
    }

    // Get slot pointer
    uint32_t slot_idx = static_cast<uint32_t>(read_idx & slot_count_mask_);
    uint8_t* slot_ptr = slots_ + (static_cast<size_t>(slot_idx) * slot_size_);

    // Read slot header: size(4) + sequence(8) + timestamp(8) = 20 bytes
    uint32_t size;
    uint64_t sequence;
    uint64_t timestamp_ns;
    std::memcpy(&size, slot_ptr + 0, sizeof(uint32_t));
    std::memcpy(&sequence, slot_ptr + 4, sizeof(uint64_t));
    std::memcpy(&timestamp_ns, slot_ptr + 12, sizeof(uint64_t));

    // Verify data not overwritten during read
    if (sequence != read_idx) {
        // Data was overwritten, skip ahead
        header_->read_idx[slot].value.store(write_idx - header_->slot_count, std::memory_order_relaxed);
        return std::nullopt;  // caller retries
    }

    // Advance read index
    header_->read_idx[slot].value.store(read_idx + 1, std::memory_order_release);

    return ReadResult{
        .data = slot_ptr + SLOT_HEADER_SIZE,
        .size = size,
        .sequence = sequence,
        .timestamp_ns = timestamp_ns
    };
}

std::optional<ReadResult> RingBufferReader::wait(int slot) {
    while (true) {
        // Try read first
        if (auto result = try_read(slot)) {
            return result;
        }

        // Load current futex value
        uint32_t current = header_->futex_word.load(std::memory_order_acquire);

        // Double-check after loading futex
        if (auto result = try_read(slot)) {
            return result;
        }

        // Block until futex changes
        futex_wait(&header_->futex_word, current);
    }
}

std::optional<ReadResult> RingBufferReader::wait_for(int slot, std::chrono::nanoseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true) {
        // Try read first
        if (auto result = try_read(slot)) {
            return result;
        }

        // Check timeout
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            return std::nullopt;
        }

        auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(deadline - now);

        // Load current futex value
        uint32_t current = header_->futex_word.load(std::memory_order_acquire);

        // Double-check after loading futex
        if (auto result = try_read(slot)) {
            return result;
        }

        // Block until futex changes or timeout
        futex_wait(&header_->futex_word, current, remaining);
    }
}

}  // namespace internal
}  // namespace conduit
