#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace conduit {
namespace internal {

constexpr size_t CACHE_LINE_SIZE = 64;
constexpr size_t MAX_SUBSCRIBERS = 16;
// Slot layout: size(4) + sequence(8) + timestamp_ns(8) = 20 bytes
constexpr size_t SLOT_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);  // 20 bytes

struct RingBufferConfig {
    uint32_t slot_count;    // must be power of 2
    uint32_t slot_size;     // bytes per slot (including slot header)
};

struct ReadResult {
    const void* data;       // pointer to payload
    size_t size;            // payload size
    uint64_t sequence;      // message sequence number
    uint64_t timestamp_ns;  // CLOCK_MONOTONIC_RAW nanoseconds
};

struct alignas(CACHE_LINE_SIZE) AlignedAtomicU64 {
    std::atomic<uint64_t> value;
};

struct RingBufferHeader {
    // Configuration (immutable after init)
    uint32_t slot_count;
    uint32_t slot_size;
    uint32_t max_subscribers;
    uint32_t padding0;

    // Writer state (own cache line)
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> write_idx;

    // Subscriber management (own cache line)
    alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> subscriber_mask;
    std::atomic<uint32_t> futex_word;

    // Reader state (each on own cache line)
    alignas(CACHE_LINE_SIZE) AlignedAtomicU64 read_idx[MAX_SUBSCRIBERS];
};

inline bool is_power_of_two(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

inline size_t calculate_region_size(const RingBufferConfig& config) {
    return sizeof(RingBufferHeader) + static_cast<size_t>(config.slot_count) * config.slot_size;
}

class RingBufferWriter {
public:
    RingBufferWriter(void* region, size_t region_size, const RingBufferConfig& config);

    void initialize();
    bool try_write(const void* data, size_t len);
    RingBufferHeader* header() { return header_; }

private:
    RingBufferHeader* header_;
    uint8_t* slots_;
    uint32_t slot_size_;
    uint32_t slot_count_;
    uint32_t slot_count_mask_;
};

class RingBufferReader {
public:
    RingBufferReader(void* region, size_t region_size);

    int claim_slot();
    void release_slot(int slot);

    // Non-blocking read
    std::optional<ReadResult> try_read(int slot);

    // Blocking read (waits forever)
    std::optional<ReadResult> wait(int slot);

    // Blocking read with timeout
    std::optional<ReadResult> wait_for(int slot, std::chrono::nanoseconds timeout);

    RingBufferHeader* header() { return header_; }

private:
    RingBufferHeader* header_;
    uint8_t* slots_;
    uint32_t slot_size_;
    uint32_t slot_count_mask_;
};

}  // namespace internal
}  // namespace conduit
