/**
 * @file ring_buffer.cpp
 * @brief Lock-free Ring Buffer - The message queue living in shared memory
 *
 * == What is a Ring Buffer? ==
 *
 * A ring buffer is a fixed-size circular queue. When you reach the end,
 * you wrap around to the beginning:
 *
 *   Slots:  [0] [1] [2] [3] [4] [5] [6] [7]   (8 slots)
 *            ^                           ^
 *            └── write wraps here ───────┘
 *
 * This is perfect for streaming data where:
 * - We have continuous new data (sensor readings, camera frames)
 * - Old data becomes stale and can be overwritten
 * - We need fixed memory usage (no malloc during operation)
 *
 * == Memory Layout ==
 *
 * The shared memory region is organized as:
 *
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │                    RingBufferHeader (1.1KB)                     │
 *   │  - slot_count, slot_size (config)                               │
 *   │  - write_idx (publisher's position)                             │
 *   │  - read_idx[16] (each subscriber's position)                    │
 *   │  - subscriber_mask (which slots are taken)                      │
 *   │  - futex_word (for sleep/wake signaling)                        │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │                         Slot 0                                  │
 *   │  [size:4B][sequence:8B][timestamp:8B][payload:...]              │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │                         Slot 1                                  │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │                          ...                                    │
 *   ├─────────────────────────────────────────────────────────────────┤
 *   │                       Slot N-1                                  │
 *   └─────────────────────────────────────────────────────────────────┘
 *
 * == How an int (4 bytes) flows through ==
 *
 * Publisher writes int value = 42:
 *
 *   1. Check if 4 bytes fits in slot (4 + 20 header = 24 bytes, yes!)
 *   2. Calculate slot index: write_idx % slot_count
 *      e.g., write_idx=5, slot_count=8 -> slot 5
 *   3. Write to slot 5:
 *      - bytes 0-3:   size = 4
 *      - bytes 4-11:  sequence = 5
 *      - bytes 12-19: timestamp = 1234567890
 *      - bytes 20-23: the int value 42
 *   4. Increment write_idx: 5 -> 6
 *   5. Wake any sleeping subscribers
 *
 * Subscriber reads:
 *   1. Compare read_idx (5) with write_idx (6) -> data available!
 *   2. Read from slot 5 (read_idx % slot_count)
 *   3. Verify sequence matches (5 == 5, data wasn't overwritten)
 *   4. Return pointer to bytes 20-23, size 4
 *   5. Increment read_idx: 5 -> 6
 *
 * == How a PointCloud (12MB) flows through ==
 *
 * Same process, just more bytes:
 *
 *   Slot must be big enough: slot_size >= 12MB + 20 byte header
 *   Publisher:
 *     1. memcpy 12MB payload into slot
 *     2. Write header (size=12MB, sequence, timestamp)
 *     3. Increment write_idx, wake subscribers
 *
 *   Subscriber:
 *     1. Gets pointer directly into shared memory
 *     2. NO COPY - just reads the 12MB from that pointer
 *
 * == Lock-Free Design ==
 *
 * No mutexes! Coordination uses:
 * - Atomic operations (load/store with memory ordering)
 * - Sequence numbers (to detect overwritten data)
 *
 * This means:
 * - No deadlocks possible
 * - Crashed processes don't block others
 * - Single writer, multiple readers (SPMC pattern)
 *
 * == Cache Line Alignment ==
 *
 * Each reader's read_idx is on its own 64-byte cache line.
 * This prevents "false sharing" - CPUs don't fight over the same cache line.
 */

#include "conduit_core/internal/ring_buffer.hpp"
#include "conduit_core/internal/futex.hpp"
#include "conduit_core/internal/time.hpp"

#include <cassert>
#include <cstring>

namespace conduit {
namespace internal {

// ============================================================================
// RingBufferWriter - Used by Publisher
// ============================================================================

/**
 * Constructor - sets up writer to use existing shared memory region.
 *
 * @param region       Pointer to shared memory (from ShmRegion::data())
 * @param region_size  Size of the region in bytes
 * @param config       Ring buffer configuration (slot count/size)
 *
 * Memory layout:
 *   region points to: [Header][Slot0][Slot1]...[SlotN-1]
 *
 * The slot_count_mask_ is for fast modulo: idx & mask == idx % slot_count
 * This only works when slot_count is a power of 2 (e.g., 8, 16, 32).
 */
RingBufferWriter::RingBufferWriter(void* region, size_t region_size, const RingBufferConfig& config)
    : header_(static_cast<RingBufferHeader*>(region)),
      slots_(static_cast<uint8_t*>(region) + sizeof(RingBufferHeader)),
      slot_size_(config.slot_size),
      slot_count_(config.slot_count),
      slot_count_mask_(config.slot_count - 1) {

    // Verify configuration
    assert(is_power_of_two(config.slot_count));  // Required for fast modulo
    assert(region_size >= calculate_region_size(config));  // Region big enough
}

/**
 * Initialize the ring buffer header.
 *
 * Called once by publisher when creating a new topic.
 * Sets up configuration and zeros all indices.
 *
 * The memory_order_release fence ensures all writes are visible
 * to other processes before they can read the header.
 */
void RingBufferWriter::initialize() {
    // Write configuration (immutable after init)
    header_->slot_count = slot_count_;
    header_->slot_size = slot_size_;
    header_->max_subscribers = MAX_SUBSCRIBERS;
    header_->padding0 = 0;

    // Initialize indices to 0
    header_->write_idx.store(0, std::memory_order_relaxed);
    header_->subscriber_mask.store(0, std::memory_order_relaxed);
    header_->futex_word.store(0, std::memory_order_relaxed);

    // Initialize all reader positions to 0
    for (size_t i = 0; i < MAX_SUBSCRIBERS; ++i) {
        header_->read_idx[i].value.store(0, std::memory_order_relaxed);
    }

    // Ensure all initializations are visible before anyone reads
    std::atomic_thread_fence(std::memory_order_release);
}

/**
 * Write a message to the ring buffer.
 *
 * @param data  Pointer to message data
 * @param len   Size of message in bytes
 * @return      true if written, false if message too large
 *
 * This is the hot path for publishing. Steps:
 *
 * 1. SIZE CHECK
 *    Verify payload + header fits in slot
 *
 * 2. CALCULATE SLOT
 *    Use write_idx to find which slot to write to
 *    slot_index = write_idx % slot_count (using bitmask for speed)
 *
 * 3. WRITE SLOT HEADER
 *    - size: how many bytes of payload
 *    - sequence: equals write_idx (for detecting overwrites)
 *    - timestamp: nanoseconds since boot
 *
 * 4. WRITE PAYLOAD
 *    memcpy the actual message data
 *
 * 5. PUBLISH (make visible to readers)
 *    Increment write_idx with release ordering
 *    This ensures steps 3-4 are visible before step 5
 *
 * 6. WAKE SUBSCRIBERS
 *    Signal anyone sleeping via futex
 */
bool RingBufferWriter::try_write(const void* data, size_t len) {
    // Step 1: Check message fits in slot
    // Slot layout: [header:20 bytes][payload:len bytes]
    if (len + SLOT_HEADER_SIZE > slot_size_) {
        return false;  // Message too large for configured slot size
    }

    // Step 2: Get current write position and calculate slot
    uint64_t idx = header_->write_idx.load(std::memory_order_relaxed);

    // Fast modulo using bitmask (works because slot_count is power of 2)
    // e.g., idx=13, slot_count=8 -> 13 & 7 = 5
    uint32_t slot_idx = static_cast<uint32_t>(idx & slot_count_mask_);

    // Pointer to this slot's memory
    uint8_t* slot_ptr = slots_ + (static_cast<size_t>(slot_idx) * slot_size_);

    // Step 3: Get timestamp for this message
    uint64_t timestamp_ns = get_timestamp_ns();

    // Step 4: Write slot header
    // Layout: [size:4B @ offset 0][sequence:8B @ offset 4][timestamp:8B @ offset 12]
    uint32_t size32 = static_cast<uint32_t>(len);
    std::memcpy(slot_ptr + 0, &size32, sizeof(uint32_t));      // bytes 0-3
    std::memcpy(slot_ptr + 4, &idx, sizeof(uint64_t));         // bytes 4-11
    std::memcpy(slot_ptr + 12, &timestamp_ns, sizeof(uint64_t)); // bytes 12-19

    // Step 5: Write payload data after header
    std::memcpy(slot_ptr + SLOT_HEADER_SIZE, data, len);       // bytes 20+

    // Step 6: Publish - increment write_idx
    // memory_order_release ensures all writes above are visible
    // to other threads/processes before they see the new write_idx
    header_->write_idx.store(idx + 1, std::memory_order_release);

    // Step 7: Wake any subscribers waiting for data
    // Increment futex word (gives waiters something to compare against)
    header_->futex_word.fetch_add(1, std::memory_order_release);
    futex_wake_all(&header_->futex_word);  // Wake all sleeping subscribers

    return true;
}

// ============================================================================
// RingBufferReader - Used by Subscriber
// ============================================================================

/**
 * Constructor - sets up reader to use existing shared memory region.
 *
 * Unlike writer, reader gets config FROM the header (publisher set it).
 */
RingBufferReader::RingBufferReader(void* region, size_t region_size)
    : header_(static_cast<RingBufferHeader*>(region)),
      slots_(static_cast<uint8_t*>(region) + sizeof(RingBufferHeader)),
      slot_size_(header_->slot_size),
      slot_count_mask_(header_->slot_count - 1) {
    (void)region_size;  // Could add debug assertions here
}

/**
 * Claim a subscriber slot.
 *
 * Each subscriber needs its own slot (0-15) to track read position.
 * Uses atomic compare-and-swap to safely claim a slot.
 *
 * @return  Slot number (0-15) or -1 if all slots taken
 *
 * The subscriber_mask is a 32-bit bitmap:
 *   bit 0 = slot 0 claimed
 *   bit 1 = slot 1 claimed
 *   etc.
 *
 * Algorithm:
 * 1. Load current mask
 * 2. Find first 0 bit (unclaimed slot)
 * 3. Try to set that bit atomically
 * 4. If CAS fails (someone else claimed it), retry
 */
int RingBufferReader::claim_slot() {
    while (true) {
        // Load current subscriber bitmap
        uint32_t mask = header_->subscriber_mask.load(std::memory_order_acquire);

        // Find first free slot (first 0 bit)
        for (uint32_t i = 0; i < header_->max_subscribers; ++i) {
            if (!(mask & (1u << i))) {
                // Slot i is free, try to claim it
                uint32_t new_mask = mask | (1u << i);

                // Atomic compare-and-swap
                if (header_->subscriber_mask.compare_exchange_weak(
                        mask, new_mask,
                        std::memory_order_acq_rel,
                        std::memory_order_acquire)) {

                    // Successfully claimed slot i!
                    // Initialize read position to current write position
                    // (start reading from next message, not historical ones)
                    uint64_t write_idx = header_->write_idx.load(std::memory_order_acquire);
                    header_->read_idx[i].value.store(write_idx, std::memory_order_release);

                    return static_cast<int>(i);
                }

                // CAS failed - someone else modified mask, retry from top
                break;
            }
        }

        // Check if all slots are taken
        if (mask == (1u << header_->max_subscribers) - 1) {
            return -1;  // No slots available
        }

        // Retry - some slot became available or we lost the race
    }
}

/**
 * Release a subscriber slot.
 *
 * Called when subscriber shuts down. Clears the bit in subscriber_mask.
 */
void RingBufferReader::release_slot(int slot) {
    uint32_t bit = 1u << static_cast<uint32_t>(slot);
    header_->subscriber_mask.fetch_and(~bit, std::memory_order_release);
}

/**
 * Try to read the next message (non-blocking).
 *
 * @param slot  Subscriber slot number (from claim_slot)
 * @return      ReadResult with pointer to data, or nullopt if no data
 *
 * Steps:
 *
 * 1. CHECK FOR DATA
 *    Compare read_idx with write_idx
 *
 * 2. CHECK FOR OVERRUN
 *    If publisher has lapped us, skip to oldest available
 *
 * 3. READ SLOT
 *    Get pointer to slot, read header
 *
 * 4. VERIFY NOT OVERWRITTEN
 *    Sequence in slot should match our read_idx
 *    If not, data was overwritten while we were reading
 *
 * 5. RETURN RESULT
 *    Pointer directly into shared memory (zero-copy!)
 */
std::optional<ReadResult> RingBufferReader::try_read(int slot) {
    // Step 1: Load our read position and publisher's write position
    uint64_t read_idx = header_->read_idx[slot].value.load(std::memory_order_relaxed);
    uint64_t write_idx = header_->write_idx.load(std::memory_order_acquire);

    // Check if there's data to read
    if (read_idx >= write_idx) {
        return std::nullopt;  // No new messages
    }

    // Step 2: Check if we've fallen behind (publisher overwrote our data)
    // This happens if: (write_idx - read_idx) > slot_count
    // i.e., publisher has written more than one full buffer since we last read
    if (write_idx - read_idx > header_->slot_count) {
        // Skip to oldest available data
        read_idx = write_idx - header_->slot_count;
        header_->read_idx[slot].value.store(read_idx, std::memory_order_relaxed);
    }

    // Step 3: Calculate slot pointer
    uint32_t slot_idx = static_cast<uint32_t>(read_idx & slot_count_mask_);
    uint8_t* slot_ptr = slots_ + (static_cast<size_t>(slot_idx) * slot_size_);

    // Read slot header
    uint32_t size;
    uint64_t sequence;
    uint64_t timestamp_ns;
    std::memcpy(&size, slot_ptr + 0, sizeof(uint32_t));
    std::memcpy(&sequence, slot_ptr + 4, sizeof(uint64_t));
    std::memcpy(&timestamp_ns, slot_ptr + 12, sizeof(uint64_t));

    // Step 4: Verify data wasn't overwritten during our read
    // The sequence number should equal our read_idx
    if (sequence != read_idx) {
        // Data was overwritten! Skip to oldest available and retry
        header_->read_idx[slot].value.store(write_idx - header_->slot_count, std::memory_order_relaxed);
        return std::nullopt;  // Caller will retry
    }

    // Step 5: Success! Advance read position
    header_->read_idx[slot].value.store(read_idx + 1, std::memory_order_release);

    // Return pointer directly into shared memory (ZERO COPY!)
    return ReadResult{
        .data = slot_ptr + SLOT_HEADER_SIZE,  // Pointer to payload
        .size = size,                          // Payload size
        .sequence = sequence,                  // For debugging/ordering
        .timestamp_ns = timestamp_ns           // When published
    };
}

/**
 * Wait for and read the next message (blocking).
 *
 * Uses futex for efficient sleeping - zero CPU when no data.
 * The futex_wait call puts the thread to sleep until woken by publisher.
 *
 * @param slot  Subscriber slot number
 * @return      ReadResult (always valid, waits forever)
 */
std::optional<ReadResult> RingBufferReader::wait(int slot) {
    while (true) {
        // Try non-blocking read first
        if (auto result = try_read(slot)) {
            return result;
        }

        // No data - prepare to sleep
        // Load futex word BEFORE checking for data again
        uint32_t current = header_->futex_word.load(std::memory_order_acquire);

        // Double-check: data might have arrived between try_read and futex load
        if (auto result = try_read(slot)) {
            return result;
        }

        // Sleep until futex word changes (publisher increments it)
        // This is a Linux system call that puts thread to sleep efficiently
        futex_wait(&header_->futex_word, current);

        // Woken up - loop back and try to read
    }
}

/**
 * Wait for message with timeout.
 *
 * Same as wait() but gives up after timeout.
 *
 * @param slot     Subscriber slot number
 * @param timeout  Maximum time to wait
 * @return         ReadResult, or nullopt if timeout
 */
std::optional<ReadResult> RingBufferReader::wait_for(int slot, std::chrono::nanoseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;

    while (true) {
        // Try non-blocking read
        if (auto result = try_read(slot)) {
            return result;
        }

        // Check timeout
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            return std::nullopt;  // Timed out
        }

        auto remaining = std::chrono::duration_cast<std::chrono::nanoseconds>(deadline - now);

        // Load futex and double-check
        uint32_t current = header_->futex_word.load(std::memory_order_acquire);

        if (auto result = try_read(slot)) {
            return result;
        }

        // Sleep with timeout
        futex_wait(&header_->futex_word, current, remaining);
    }
}

}  // namespace internal
}  // namespace conduit
