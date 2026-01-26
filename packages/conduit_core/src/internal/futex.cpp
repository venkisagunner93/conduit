/**
 * @file futex.cpp
 * @brief Futex - Fast Userspace Mutex for efficient thread/process signaling
 *
 * == What is a Futex? ==
 *
 * Futex = "Fast Userspace muTEX"
 *
 * It's a Linux kernel feature for efficient waiting. The key insight:
 *
 *   Most of the time, there's no contention. We don't need the kernel.
 *   But when we DO need to wait, we want to sleep efficiently.
 *
 * Futex gives us both:
 * - Fast path: Just check a number in memory (no kernel call)
 * - Slow path: Sleep in kernel until woken (no CPU usage)
 *
 * == How Conduit Uses Futex ==
 *
 * The ring buffer has a "futex_word" - just a uint32_t counter.
 *
 * Publisher (after writing data):
 *   1. Increment futex_word
 *   2. Call futex_wake_all() to wake sleeping subscribers
 *
 * Subscriber (when no data available):
 *   1. Load futex_word (e.g., value = 5)
 *   2. Double-check there's really no data
 *   3. Call futex_wait(word, 5)
 *      - If word is still 5: sleep until woken
 *      - If word changed: return immediately (data arrived!)
 *
 * This is much more efficient than:
 * - Busy-waiting (while(no_data) {}) - burns 100% CPU
 * - Sleep polling (while(no_data) sleep(10ms)) - adds latency
 *
 * With futex:
 * - Zero CPU when no data (thread is truly asleep)
 * - Instant wake-up when data arrives (kernel directly wakes thread)
 *
 * == Why Not Use a Mutex? ==
 *
 * A mutex (pthread_mutex) would work, but:
 * - More overhead (locks, unlocks, even when no contention)
 * - Doesn't work well across processes (needs special setup)
 * - If a process crashes holding the lock, others deadlock
 *
 * Futex is simpler for this use case:
 * - Just a number in shared memory
 * - No "lock ownership" - crashed processes don't block others
 * - Works across processes by default
 */

#include "conduit_core/internal/futex.hpp"

#include <linux/futex.h>  // FUTEX_WAIT, FUTEX_WAKE
#include <sys/syscall.h>  // SYS_futex
#include <unistd.h>       // syscall()
#include <cerrno>         // errno, EAGAIN, ETIMEDOUT
#include <climits>        // INT_MAX

namespace conduit::internal {

/**
 * Wait until futex_word changes from expected_value.
 *
 * @param futex_word      Pointer to the 32-bit word to watch
 * @param expected_value  Sleep only if word equals this value
 * @param timeout         Optional maximum time to wait
 * @return                true if woken, false if timeout
 *
 * How it works:
 *
 * 1. Kernel checks: *futex_word == expected_value?
 *    - If NO: Return immediately (value already changed!)
 *    - If YES: Put this thread to sleep
 *
 * 2. Thread sleeps until:
 *    - Another thread calls futex_wake() on this address
 *    - Timeout expires (if specified)
 *    - Signal interrupts the wait (rare)
 *
 * The check-and-sleep is ATOMIC in the kernel. This prevents races:
 *
 *   Thread A: if (no_data) futex_wait(...)
 *   Thread B: write_data(); futex_wake(...)
 *
 * Without atomic check-and-sleep:
 *   A: checks no_data (true)
 *   B: writes data
 *   B: calls wake (but A isn't sleeping yet!)
 *   A: goes to sleep (misses the wake forever!)
 *
 * With futex:
 *   A: checks no_data (true)
 *   B: writes data, increments futex_word
 *   B: calls wake
 *   A: futex_wait sees word changed, returns immediately
 *
 * The "expected_value" makes this work - we only sleep if nothing changed.
 */
bool futex_wait(
    std::atomic<uint32_t>* futex_word,
    uint32_t expected_value,
    std::optional<std::chrono::nanoseconds> timeout
) {
    // Get raw pointer (futex syscall takes uint32_t*)
    auto* ptr = reinterpret_cast<uint32_t*>(futex_word);

    long result;

    if (timeout.has_value()) {
        // Wait with timeout
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(timeout->count() / 1'000'000'000);
        ts.tv_nsec = static_cast<long>(timeout->count() % 1'000'000'000);

        // FUTEX_WAIT: sleep if *ptr == expected_value, wake after timeout
        result = syscall(SYS_futex, ptr, FUTEX_WAIT, expected_value, &ts, nullptr, 0);
    } else {
        // Wait forever (no timeout)
        result = syscall(SYS_futex, ptr, FUTEX_WAIT, expected_value, nullptr, nullptr, 0);
    }

    // Handle result
    if (result == -1) {
        if (errno == EAGAIN) {
            // Value changed before we could sleep - this is fine!
            // It means data arrived, which is what we wanted
            return true;
        }
        if (errno == ETIMEDOUT) {
            // Timeout expired, no wake
            return false;
        }
        if (errno == EINTR) {
            // Interrupted by signal - treat as spurious wakeup
            // Caller will loop and check for data
            return true;
        }
        // Other errors: treat as woken (caller will check for data)
    }

    return true;  // Woken by futex_wake
}

/**
 * Wake up waiting threads.
 *
 * @param futex_word  Pointer to the futex word
 * @param count       Maximum number of threads to wake
 * @return            Number of threads actually woken
 *
 * This is called by the publisher after writing new data.
 * It wakes threads sleeping in futex_wait() on this address.
 *
 * Note: If no threads are waiting, this is a no-op (very cheap).
 */
int futex_wake(std::atomic<uint32_t>* futex_word, int count) {
    auto* ptr = reinterpret_cast<uint32_t*>(futex_word);

    // FUTEX_WAKE: wake up to `count` threads waiting on this address
    long result = syscall(SYS_futex, ptr, FUTEX_WAKE, count, nullptr, nullptr, 0);

    return result > 0 ? static_cast<int>(result) : 0;
}

/**
 * Wake ALL waiting threads.
 *
 * Used by publisher to wake all subscribers at once.
 * INT_MAX means "wake everyone" - the kernel stops when no more waiters.
 */
int futex_wake_all(std::atomic<uint32_t>* futex_word) {
    return futex_wake(futex_word, INT_MAX);
}

}  // namespace conduit::internal
