#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>

/// @brief Internal implementation details for conduit.
namespace conduit::internal {

/// @brief Wait until the futex word changes from the expected value.
///
/// Wraps the Linux `futex(FUTEX_WAIT)` syscall. The calling thread sleeps
/// (consuming zero CPU) until another thread calls futex_wake() on the same
/// word, or the optional timeout expires. A spurious wakeup is possible if
/// the futex word has already changed by the time the syscall executes.
///
/// @param futex_word Pointer to the atomic futex word in shared memory.
/// @param expected_value The value that triggered the wait; if the current
///        value differs, the call returns immediately.
/// @param timeout Optional maximum wait duration. std::nullopt means wait forever.
/// @return true if woken by futex_wake(), false on timeout.
bool futex_wait(
    std::atomic<uint32_t>* futex_word,
    uint32_t expected_value,
    std::optional<std::chrono::nanoseconds> timeout = std::nullopt
);

/// @brief Wake up to @p count threads waiting on the futex word.
///
/// Wraps the Linux `futex(FUTEX_WAKE)` syscall.
///
/// @param futex_word Pointer to the atomic futex word.
/// @param count Maximum number of waiters to wake (default 1).
/// @return Number of waiters actually woken.
int futex_wake(std::atomic<uint32_t>* futex_word, int count = 1);

/// @brief Wake all threads waiting on the futex word.
/// @param futex_word Pointer to the atomic futex word.
/// @return Number of waiters actually woken.
int futex_wake_all(std::atomic<uint32_t>* futex_word);

}  // namespace conduit::internal
