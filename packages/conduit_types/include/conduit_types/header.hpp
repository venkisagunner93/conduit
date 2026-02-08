#pragma once

#include <cstdint>
#include <cstring>

namespace conduit {

/// @brief Common message header with timestamp and coordinate frame.
struct Header {
    uint64_t timestamp_ns;  ///< Timestamp in nanoseconds.
    char frame[64];         ///< Coordinate frame identifier (null-terminated).
};

/// @brief Safely copy a frame string into a Header frame field.
/// @param dst Destination frame array (64 bytes, will be null-terminated).
/// @param src Source null-terminated string.
inline void set_frame(char (&dst)[64], const char* src) {
    std::strncpy(dst, src, 63);
    dst[63] = '\0';
}

}  // namespace conduit
