#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>

namespace conduit::internal {

// Wait until *futex_word != expected_value, or timeout
// Returns: true if woken, false if timeout
bool futex_wait(
    std::atomic<uint32_t>* futex_word,
    uint32_t expected_value,
    std::optional<std::chrono::nanoseconds> timeout = std::nullopt
);

// Wake up to `count` waiters
// Returns: number of waiters woken
int futex_wake(std::atomic<uint32_t>* futex_word, int count = 1);

// Wake all waiters
int futex_wake_all(std::atomic<uint32_t>* futex_word);

}  // namespace conduit::internal
