# Ring Buffer

The ring buffer is a lock-free circular queue that stores messages in shared memory.

## Overview

A ring buffer is a fixed-size circular queue. When you reach the end, you wrap around:

```
Slots:  [0] [1] [2] [3] [4] [5] [6] [7]   (8 slots)
         ^                           ^
         └── write wraps here ───────┘
```

This is ideal for streaming data where:

- New data arrives continuously
- Old data becomes stale
- Memory usage must be bounded

## Memory Layout

```
┌─────────────────────────────────────────────────────────────────┐
│                    RingBufferHeader (~1.1KB)                    │
│  - slot_count, slot_size (configuration)                        │
│  - write_idx (publisher's position)                             │
│  - read_idx[16] (each subscriber's position)                    │
│  - subscriber_mask (which slots are claimed)                    │
│  - futex_word (for sleep/wake signaling)                        │
├─────────────────────────────────────────────────────────────────┤
│                         Slot 0                                  │
│  [size: 4B] [sequence: 8B] [timestamp: 8B] [payload: ...]       │
├─────────────────────────────────────────────────────────────────┤
│                         Slot 1                                  │
├─────────────────────────────────────────────────────────────────┤
│                          ...                                    │
├─────────────────────────────────────────────────────────────────┤
│                       Slot N-1                                  │
└─────────────────────────────────────────────────────────────────┘
```

## Slot Header

Each message slot has a 20-byte header:

| Field | Size | Description |
|-------|------|-------------|
| size | 4 bytes | Payload size |
| sequence | 8 bytes | Monotonic counter (for overwrite detection) |
| timestamp | 8 bytes | Nanoseconds (CLOCK_MONOTONIC_RAW) |

## Configuration

```cpp
RingBufferConfig config;
config.slot_count = 16;     // Number of slots (must be power of 2)
config.slot_size = 4096;    // Bytes per slot (header + payload)
```

**Default values:**

- `slot_count`: 16
- `slot_size`: 4096 bytes (4KB)
- `max_subscribers`: 16

For large messages (images, point clouds), increase `slot_size`:

```cpp
config.slot_size = 13'000'000;  // 13MB for 12MB payloads
```

## Lock-Free Design

No mutexes! Coordination uses:

1. **Atomic operations** - `std::atomic<uint64_t>` for indices
2. **Memory ordering** - Ensures visibility across CPUs
3. **Sequence numbers** - Detect overwritten data

### Why Lock-Free?

- **No deadlocks** - Impossible without locks
- **Crash resilient** - Crashed process doesn't block others
- **Low latency** - No lock contention overhead

## Writing (Publisher)

```cpp
bool success = writer.try_write(data, len);
```

Steps:

1. Check size fits in slot
2. Calculate slot: `write_idx % slot_count`
3. Write header (size, sequence, timestamp)
4. Write payload
5. Increment `write_idx` (atomic, release ordering)
6. Wake sleeping subscribers (futex)

## Reading (Subscriber)

```cpp
std::optional<ReadResult> result = reader.try_read(slot);
// or
std::optional<ReadResult> result = reader.wait(slot);
```

Steps:

1. Compare `read_idx` with `write_idx`
2. If no data: return nullopt (try_read) or sleep (wait)
3. Calculate slot: `read_idx % slot_count`
4. Read header
5. **Verify sequence** - Detect if overwritten
6. Return pointer to payload (zero-copy!)
7. Increment `read_idx`

## Overwrite Detection

The sequence number detects torn reads:

```
Publisher writes slot 5 with sequence=5
Subscriber reads slot 5, verifies sequence==5 ✓

Later, publisher wraps around, writes slot 5 with sequence=21
Subscriber was slow, still at slot 5
Subscriber reads, sees sequence=21 ≠ 5 → data was overwritten!
```

When overwrite is detected, subscriber skips to newest available data.

## Multiple Subscribers

Each subscriber claims a slot (0-15) via `claim_slot()`:

```cpp
int my_slot = reader.claim_slot();  // Returns 0-15, or -1 if full
// ... use my_slot for all reads ...
reader.release_slot(my_slot);       // On shutdown
```

Each slot has its own `read_idx`, so subscribers progress independently.

## Cache Line Alignment

Each `read_idx` is on its own 64-byte cache line:

```cpp
struct alignas(64) CacheLineAligned {
    std::atomic<uint64_t> value;
};

CacheLineAligned read_idx[MAX_SUBSCRIBERS];
```

This prevents **false sharing** - CPUs don't fight over the same cache line when updating different read indices.

## Calculating Region Size

```cpp
size_t size = calculate_region_size(config);
// = sizeof(RingBufferHeader) + slot_count * slot_size
```

Example: 16 slots × 4KB = 64KB + ~1KB header ≈ 65KB per topic.
