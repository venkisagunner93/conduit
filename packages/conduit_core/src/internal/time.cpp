#include "conduit_core/internal/time.hpp"

#include <time.h>

namespace conduit::internal {

uint64_t get_timestamp_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL
         + static_cast<uint64_t>(ts.tv_nsec);
}

}  // namespace conduit::internal
