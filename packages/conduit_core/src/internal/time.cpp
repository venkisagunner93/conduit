/**
 * @file time.cpp
 * @brief High-resolution monotonic timestamp
 *
 * == Why CLOCK_MONOTONIC_RAW? ==
 *
 * Linux has several clocks:
 *
 * CLOCK_REALTIME
 *   - Wall clock time (e.g., "2024-01-15 10:30:00")
 *   - Can jump backwards! (NTP adjustment, user changing time)
 *   - Bad for measuring durations
 *
 * CLOCK_MONOTONIC
 *   - Seconds since boot (never goes backwards)
 *   - NTP can speed up/slow down the clock slightly
 *   - Good for durations, but may drift relative to hardware
 *
 * CLOCK_MONOTONIC_RAW
 *   - Raw hardware counter, not adjusted by NTP
 *   - Most accurate for short-term measurements
 *   - Ideal for latency measurements between processes
 *
 * == How It's Used ==
 *
 * Every message published gets a timestamp:
 *
 *   Publisher                          Subscriber
 *   ─────────                          ──────────
 *   get_timestamp_ns() -> 1234567890
 *   write message to slot
 *   (message includes timestamp)
 *                                      read message
 *                                      get_timestamp_ns() -> 1234568100
 *                                      latency = 1234568100 - 1234567890
 *                                             = 210 nanoseconds
 *
 * Since both processes use the same clock (kernel provides it),
 * the timestamps are comparable across processes.
 *
 * == Precision ==
 *
 * Nanosecond resolution (10^-9 seconds).
 * Typical call overhead: ~20-30 nanoseconds on modern Linux.
 *
 * The timestamp wraps around after ~584 years (2^64 nanoseconds),
 * so no overflow concerns in practice.
 */

#include "conduit_core/internal/time.hpp"

#include <time.h>  // clock_gettime, CLOCK_MONOTONIC_RAW

namespace conduit::internal {

/**
 * Get current timestamp in nanoseconds.
 *
 * Uses CLOCK_MONOTONIC_RAW for accurate, comparable timestamps
 * across all processes on the same machine.
 *
 * @return Nanoseconds since boot (monotonically increasing)
 */
uint64_t get_timestamp_ns() {
    struct timespec ts;

    // Get time from kernel
    // CLOCK_MONOTONIC_RAW = raw hardware time, not adjusted by NTP
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    // Convert to nanoseconds:
    // tv_sec * 1,000,000,000 + tv_nsec
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL
         + static_cast<uint64_t>(ts.tv_nsec);
}

}  // namespace conduit::internal
