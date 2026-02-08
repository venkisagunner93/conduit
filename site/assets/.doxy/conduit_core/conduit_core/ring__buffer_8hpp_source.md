

# File ring\_buffer.hpp

[**File List**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**internal**](dir_ee09fc54beaf3ba0ab27276c1e0adb4c.md) **>** [**ring\_buffer.hpp**](ring__buffer_8hpp.md)

[Go to the documentation of this file](ring__buffer_8hpp.md)


```C++
#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace conduit {
namespace internal {

constexpr size_t CACHE_LINE_SIZE = 64;

constexpr size_t MAX_SUBSCRIBERS = 16;

constexpr size_t SLOT_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t);  // 20 bytes

struct RingBufferConfig {
    uint32_t slot_count;    
    uint32_t slot_size;     
};

struct ReadResult {
    const void* data;       
    size_t size;            
    uint64_t sequence;      
    uint64_t timestamp_ns;  
};

struct alignas(CACHE_LINE_SIZE) AlignedAtomicU64 {
    std::atomic<uint64_t> value;
};

struct RingBufferHeader {
    // Configuration (immutable after init)
    uint32_t slot_count;        
    uint32_t slot_size;         
    uint32_t max_subscribers;   
    uint32_t padding0;          

    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> write_idx;

    alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> subscriber_mask;
    std::atomic<uint32_t> futex_word;

    alignas(CACHE_LINE_SIZE) AlignedAtomicU64 read_idx[MAX_SUBSCRIBERS];
};

inline bool is_power_of_two(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

inline size_t calculate_region_size(const RingBufferConfig& config) {
    return sizeof(RingBufferHeader) + static_cast<size_t>(config.slot_count) * config.slot_size;
}

class RingBufferWriter {
public:
    RingBufferWriter(void* region, size_t region_size, const RingBufferConfig& config);

    void initialize();

    bool try_write(const void* data, size_t len);

    RingBufferHeader* header() { return header_; }

private:
    RingBufferHeader* header_;
    uint8_t* slots_;
    uint32_t slot_size_;
    uint32_t slot_count_;
    uint32_t slot_count_mask_;
};

class RingBufferReader {
public:
    RingBufferReader(void* region, size_t region_size);

    int claim_slot();

    void release_slot(int slot);

    std::optional<ReadResult> try_read(int slot);

    std::optional<ReadResult> wait(int slot);

    std::optional<ReadResult> wait_for(int slot, std::chrono::nanoseconds timeout);

    RingBufferHeader* header() { return header_; }

private:
    RingBufferHeader* header_;
    uint8_t* slots_;
    uint32_t slot_size_;
    uint32_t slot_count_mask_;
};

}  // namespace internal
}  // namespace conduit
```


