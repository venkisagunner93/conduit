#pragma once

#include <cstdint>
#include <cstring>

namespace conduit {

struct Header {
    uint64_t timestamp_ns;
    char frame[64];
};

inline void set_frame(char (&dst)[64], const char* src) {
    std::strncpy(dst, src, 63);
    dst[63] = '\0';
}

}  // namespace conduit
