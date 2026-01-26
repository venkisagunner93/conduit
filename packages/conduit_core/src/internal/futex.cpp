#include "conduit_core/internal/futex.hpp"

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cerrno>
#include <climits>

namespace conduit::internal {

bool futex_wait(
    std::atomic<uint32_t>* futex_word,
    uint32_t expected_value,
    std::optional<std::chrono::nanoseconds> timeout
) {
    auto* ptr = reinterpret_cast<uint32_t*>(futex_word);

    long result;
    if (timeout.has_value()) {
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(timeout->count() / 1'000'000'000);
        ts.tv_nsec = static_cast<long>(timeout->count() % 1'000'000'000);

        result = syscall(SYS_futex, ptr, FUTEX_WAIT, expected_value, &ts, nullptr, 0);
    } else {
        result = syscall(SYS_futex, ptr, FUTEX_WAIT, expected_value, nullptr, nullptr, 0);
    }

    if (result == -1) {
        if (errno == EAGAIN) {
            // Value changed before we could wait - not an error
            return true;
        }
        if (errno == ETIMEDOUT) {
            return false;
        }
        if (errno == EINTR) {
            // Interrupted by signal - treat as spurious wakeup
            return true;
        }
        // Other errors: treat as woken
    }

    return true;
}

int futex_wake(std::atomic<uint32_t>* futex_word, int count) {
    auto* ptr = reinterpret_cast<uint32_t*>(futex_word);
    long result = syscall(SYS_futex, ptr, FUTEX_WAKE, count, nullptr, nullptr, 0);
    return result > 0 ? static_cast<int>(result) : 0;
}

int futex_wake_all(std::atomic<uint32_t>* futex_word) {
    return futex_wake(futex_word, INT_MAX);
}

}  // namespace conduit::internal
