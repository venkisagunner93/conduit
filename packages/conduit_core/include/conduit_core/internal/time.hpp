#pragma once

#include <cstdint>

namespace conduit::internal {

/// @brief Get the current timestamp in nanoseconds from CLOCK_MONOTONIC_RAW.
/// @return Nanoseconds since an arbitrary epoch (monotonic, not wall-clock).
uint64_t get_timestamp_ns();

}  // namespace conduit::internal
