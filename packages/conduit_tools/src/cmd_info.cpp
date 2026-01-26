#include "conduit_tools/commands.hpp"
#include <conduit_core/internal/ring_buffer.hpp>
#include <conduit_core/internal/shm_region.hpp>
#include <conduit_core/log.hpp>
#include <string>

namespace conduit::tools {

int cmd_info(int argc, char** argv) {
    if (argc < 2) {
        log::error("Usage: conduit info <topic>");
        return 1;
    }

    std::string topic = argv[1];

    if (!internal::ShmRegion::exists(topic)) {
        log::error("Topic not found: {}", topic);
        return 1;
    }

    auto shm = internal::ShmRegion::open(topic);
    auto* header = static_cast<internal::RingBufferHeader*>(shm.data());

    uint32_t mask = header->subscriber_mask.load(std::memory_order_acquire);
    int sub_count = __builtin_popcount(mask);
    uint64_t write_idx = header->write_idx.load(std::memory_order_acquire);

    fmt::print("Topic:              {}\n", topic);
    fmt::print("Slot count:         {}\n", header->slot_count);
    fmt::print("Slot size:          {} bytes\n", header->slot_size);
    fmt::print("Max subscribers:    {}\n", header->max_subscribers);
    fmt::print("Active subscribers: {}\n", sub_count);
    fmt::print("Messages published: {}\n", write_idx);

    return 0;
}

}  // namespace conduit::tools
