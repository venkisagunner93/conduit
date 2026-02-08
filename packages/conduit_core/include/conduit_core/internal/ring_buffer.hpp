#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace conduit {
namespace internal {

/// CPU cache line size used for alignment to prevent false sharing.
constexpr size_t CACHE_LINE_SIZE = 64;

/// Maximum number of concurrent subscriber reader slots.
constexpr size_t MAX_SUBSCRIBERS = 16;

/// @brief Size of each slot's header in bytes.
///
/// Slot header layout:
/// @code
///   ┌────────────┬──────────────┬────────────────┐
///   │ size (4B)  │ sequence (8B)│ timestamp (8B)  │  = 20 bytes
///   └────────────┴──────────────┴────────────────┘
/// @endcode
constexpr size_t SLOT_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);  // 20 bytes

/// @brief Ring buffer configuration.
struct RingBufferConfig {
    uint32_t slot_count;    ///< Number of slots (must be power of 2).
    uint32_t slot_size;     ///< Bytes per slot (including slot header).
};

/// @brief Result of a successful ring buffer read.
struct ReadResult {
    const void* data;       ///< Pointer to payload within the slot.
    size_t size;            ///< Payload size in bytes.
    uint64_t sequence;      ///< Message sequence number.
    uint64_t timestamp_ns;  ///< CLOCK_MONOTONIC_RAW timestamp in nanoseconds.
};

/// @brief Cache-line-aligned atomic uint64_t to prevent false sharing.
struct alignas(CACHE_LINE_SIZE) AlignedAtomicU64 {
    std::atomic<uint64_t> value;
};

/// @brief Shared memory layout for the ring buffer control structure.
///
/// Resides at the start of the shared memory region, followed by the
/// slot data array. Each field group is cache-line-aligned to prevent
/// false sharing between the writer and readers.
///
/// @code
///   Shared Memory Layout:
///   ┌────────────────────────────────────────┐  offset 0
///   │  RingBufferHeader                      │
///   │  ┌──────────────────────────────────┐  │
///   │  │ config (immutable after init)    │  │
///   │  │  slot_count, slot_size, etc.     │  │
///   │  ├──────────────────────────────────┤  │  aligned 64B
///   │  │ write_idx (writer only)          │  │
///   │  ├──────────────────────────────────┤  │  aligned 64B
///   │  │ subscriber_mask + futex_word     │  │
///   │  ├──────────────────────────────────┤  │  aligned 64B
///   │  │ read_idx[0..MAX_SUBSCRIBERS-1]   │  │  each aligned 64B
///   │  └──────────────────────────────────┘  │
///   ├────────────────────────────────────────┤
///   │  Slot[0]: [hdr 20B | payload ...]     │
///   │  Slot[1]: [hdr 20B | payload ...]     │
///   │  ...                                   │
///   │  Slot[N-1]: [hdr 20B | payload ...]   │
///   └────────────────────────────────────────┘
/// @endcode
struct RingBufferHeader {
    // Configuration (immutable after init)
    uint32_t slot_count;        ///< Number of slots.
    uint32_t slot_size;         ///< Bytes per slot.
    uint32_t max_subscribers;   ///< Maximum reader slots.
    uint32_t padding0;          ///< Padding for alignment.

    /// Writer's next write index (own cache line to avoid false sharing).
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> write_idx;

    /// Bitmask of claimed subscriber slots (own cache line).
    alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> subscriber_mask;
    /// Futex word used for subscriber wake signaling.
    std::atomic<uint32_t> futex_word;

    /// Per-reader current read index (each on own cache line).
    alignas(CACHE_LINE_SIZE) AlignedAtomicU64 read_idx[MAX_SUBSCRIBERS];
};

/// @brief Check if n is a power of two.
/// @param n Value to check.
/// @return true if n is a power of two.
inline bool is_power_of_two(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

/// @brief Calculate total shared memory region size for the given config.
/// @param config Ring buffer configuration.
/// @return Total size in bytes (header + all slots).
inline size_t calculate_region_size(const RingBufferConfig& config) {
    return sizeof(RingBufferHeader) + static_cast<size_t>(config.slot_count) * config.slot_size;
}

/// @brief Writer side of the lock-free SPMC ring buffer.
///
/// There is exactly one writer per topic. The writer initializes the shared
/// memory header, then writes messages into slots in a circular pattern.
/// After each write, subscribers are woken via futex.
///
/// @see RingBufferReader
class RingBufferWriter {
public:
    /// @brief Construct a writer over a shared memory region.
    /// @param region Pointer to the shared memory region.
    /// @param region_size Total size of the region in bytes.
    /// @param config Ring buffer configuration (slot_count must be power of 2).
    RingBufferWriter(void* region, size_t region_size, const RingBufferConfig& config);

    /// @brief Initialize the ring buffer header in shared memory.
    void initialize();

    /// @brief Write a message to the next slot in the ring buffer.
    ///
    /// Automatically timestamps the message with CLOCK_MONOTONIC_RAW,
    /// increments the sequence number, and wakes waiting subscribers.
    ///
    /// @param data Pointer to the payload.
    /// @param len Payload size in bytes.
    /// @return true if written, false if len exceeds the slot's payload capacity.
    bool try_write(const void* data, size_t len);

    /// @brief Access the ring buffer header.
    /// @return Pointer to the header in shared memory.
    RingBufferHeader* header() { return header_; }

private:
    RingBufferHeader* header_;
    uint8_t* slots_;
    uint32_t slot_size_;
    uint32_t slot_count_;
    uint32_t slot_count_mask_;
};

/// @brief Reader side of the lock-free SPMC ring buffer.
///
/// Multiple readers can exist per topic (up to MAX_SUBSCRIBERS). Each reader
/// claims a slot via claim_slot(), then reads messages independently.
/// If the writer laps a reader, the reader detects the overwrite via
/// sequence number validation and skips ahead.
///
/// @see RingBufferWriter
class RingBufferReader {
public:
    /// @brief Construct a reader over an existing shared memory region.
    /// @param region Pointer to the shared memory region (must already be initialized).
    /// @param region_size Total size of the region in bytes.
    RingBufferReader(void* region, size_t region_size);

    /// @brief Claim a subscriber slot in the ring buffer.
    /// @return Slot index (0..MAX_SUBSCRIBERS-1), or -1 if all slots are taken.
    int claim_slot();

    /// @brief Release a previously claimed subscriber slot.
    /// @param slot Slot index to release.
    void release_slot(int slot);

    /// @brief Non-blocking read of the next message.
    /// @param slot Reader slot index from claim_slot().
    /// @return The next message, or std::nullopt if no new message is available.
    std::optional<ReadResult> try_read(int slot);

    /// @brief Block until a message is available (waits forever).
    ///
    /// Uses futex-based signaling for zero CPU usage while idle.
    ///
    /// @param slot Reader slot index.
    /// @return The next message, or std::nullopt on spurious wakeup.
    std::optional<ReadResult> wait(int slot);

    /// @brief Block until a message is available or timeout expires.
    /// @param slot Reader slot index.
    /// @param timeout Maximum time to wait.
    /// @return The next message, or std::nullopt on timeout.
    std::optional<ReadResult> wait_for(int slot, std::chrono::nanoseconds timeout);

    /// @brief Access the ring buffer header.
    /// @return Pointer to the header in shared memory.
    RingBufferHeader* header() { return header_; }

private:
    RingBufferHeader* header_;
    uint8_t* slots_;
    uint32_t slot_size_;
    uint32_t slot_count_mask_;
};

}  // namespace internal
}  // namespace conduit
