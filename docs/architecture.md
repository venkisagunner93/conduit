# Architecture

A deep dive into how Conduit works.

## Overview

```
┌─────────────────────────────────────────────────────────────┐
│                       Your Code                             │
│                                                             │
│   class MyNode : public conduit::Node { ... }               │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      conduit::Node                          │
│                                                             │
│   • Thread per subscription                                 │
│   • Timer threads for loops                                 │
│   • Signal handling (SIGINT/SIGTERM)                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Publisher / Subscriber                     │
│                                                             │
│   • Publisher: write to ring buffer + wake subscribers      │
│   • Subscriber: wait on futex + read from ring buffer       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Lock-Free Ring Buffer                      │
│                                                             │
│   • Single producer, multiple consumers (SPMC)              │
│   • Atomic indices, no mutexes                              │
│   • Sequence numbers for overwrite detection                │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     Shared Memory                           │
│                                                             │
│   /dev/shm/conduit_{topic}                                  │
│   • RAM-backed filesystem                                   │
│   • mmap'd into each process                                │
└─────────────────────────────────────────────────────────────┘
```

## Shared Memory

### What It Is

On Linux, `/dev/shm` is a RAM-backed filesystem (tmpfs). Files there exist only in memory.

When a publisher creates a topic:
```bash
$ ls -la /dev/shm/conduit_*
-rw-rw-rw- 1 user user 69632 Jan 15 10:30 /dev/shm/conduit_imu
```

### How It Enables Zero-Copy

Normal IPC copies data multiple times:
```
Process A → kernel buffer → Process B
```

Shared memory maps the same physical RAM into both processes:
```
Process A's address space:  [...][SHARED REGION][...]
                                      │
                            (same physical RAM)
                                      │
Process B's address space:  [...][SHARED REGION][...]
```

Publisher writes once. Subscribers read directly from that memory. No copies.

### System Calls

| Function | Purpose |
|----------|---------|
| `shm_open()` | Create/open shared memory file |
| `ftruncate()` | Set size (allocate RAM) |
| `mmap()` | Map into address space |
| `shm_unlink()` | Delete (when publisher exits) |

## Ring Buffer

### Why a Ring Buffer?

Robotics data is streaming: sensors produce continuous readings, old data becomes stale. A ring buffer:

- Has fixed memory usage (no allocation during operation)
- Automatically overwrites old data
- Supports multiple readers at different speeds

### Memory Layout

```
┌─────────────────────────────────────────────────────────────────┐
│                         Header (~1 KB)                          │
│                                                                 │
│  slot_count    slot_size    write_idx    futex_word             │
│  read_idx[0]   read_idx[1]  ...          read_idx[15]           │
│  subscriber_mask                                                │
├─────────────────────────────────────────────────────────────────┤
│  Slot 0: [size][sequence][timestamp][payload...]                │
├─────────────────────────────────────────────────────────────────┤
│  Slot 1: [size][sequence][timestamp][payload...]                │
├─────────────────────────────────────────────────────────────────┤
│  ...                                                            │
├─────────────────────────────────────────────────────────────────┤
│  Slot N-1: [size][sequence][timestamp][payload...]              │
└─────────────────────────────────────────────────────────────────┘
```

### Slot Header

Each message has a 20-byte header:

| Field | Size | Description |
|-------|------|-------------|
| `size` | 4 bytes | Payload size |
| `sequence` | 8 bytes | Monotonic counter |
| `timestamp_ns` | 8 bytes | CLOCK_MONOTONIC_RAW |

### Lock-Free Operation

No mutexes. Coordination uses:

1. **Atomic indices** — `std::atomic<uint64_t>` for write_idx and read_idx
2. **Memory ordering** — `memory_order_release` / `memory_order_acquire`
3. **Sequence numbers** — Detect if data was overwritten

**Publishing:**
```cpp
1. slot_index = write_idx % slot_count
2. Write header + payload to slot
3. write_idx.store(idx + 1, memory_order_release)  // Publish!
4. futex_wake_all()  // Wake subscribers
```

**Subscribing:**
```cpp
1. if (read_idx >= write_idx) futex_wait()  // Sleep if no data
2. slot_index = read_idx % slot_count
3. Read header, verify sequence matches read_idx
4. read_idx.store(idx + 1, memory_order_release)
5. Return pointer to payload  // Zero-copy!
```

### Overwrite Detection

The sequence number prevents reading corrupted data:

```
Publisher writes slot 5 with sequence=5
Subscriber reads slot 5, checks: sequence(5) == read_idx(5) ✓

Later, publisher wraps around, writes slot 5 with sequence=21
Slow subscriber still at read_idx=5
Subscriber reads, sees sequence=21 ≠ 5 → Data was overwritten!
Subscriber skips to newest available data
```

### Multiple Subscribers

Each subscriber claims a slot (0-15) with its own read_idx:

```
subscriber_mask: 0b00000111  (3 subscribers active)

read_idx[0] = 42  ← Subscriber A (caught up)
read_idx[1] = 40  ← Subscriber B (2 behind)
read_idx[2] = 38  ← Subscriber C (4 behind)
```

Subscribers progress independently. No load balancing — all get all messages.

## Futex Signaling

### The Problem

How should a subscriber wait for data?

**Busy-wait:** `while (!has_data) {}` — Burns 100% CPU

**Sleep-poll:** `while (!has_data) sleep(10ms)` — Adds latency

### The Solution: Futex

Futex (Fast Userspace muTEX) is a Linux kernel feature:

```cpp
// Subscriber: sleep efficiently
futex_wait(&futex_word, current_value);  // Sleeps in kernel

// Publisher: wake subscribers
futex_word++;
futex_wake_all(&futex_word);  // Kernel wakes sleepers
```

Benefits:
- **Zero CPU when idle** — Thread truly sleeps in kernel
- **Instant wake** — Kernel directly wakes the thread
- **No polling latency** — Sub-microsecond response

### How It Works

1. Subscriber loads `futex_word` value (e.g., 5)
2. Subscriber checks for data — none available
3. Subscriber calls `futex_wait(&futex_word, 5)`
   - Kernel checks: is `futex_word` still 5?
   - If yes: put thread to sleep
   - If no: return immediately (data arrived!)
4. Publisher writes data, increments `futex_word` to 6
5. Publisher calls `futex_wake_all()`
6. Kernel wakes sleeping subscriber

The check-and-sleep is atomic in the kernel, preventing races.

## Node Threading Model

```
┌─────────────────────────────────────────────────────────────┐
│                       Main Thread                           │
│                                                             │
│   node.run() — blocks here, handles signals                 │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────┐  ┌─────────────────────┐
│  Subscription       │  │  Subscription       │
│  Thread (imu)       │  │  Thread (camera)    │
│                     │  │                     │
│  while (running):   │  │  while (running):   │
│    msg = sub.wait() │  │    msg = sub.wait() │
│    callback(msg)    │  │    callback(msg)    │
└─────────────────────┘  └─────────────────────┘

┌─────────────────────┐  ┌─────────────────────┐
│  Loop Thread        │  │  Loop Thread        │
│  (100 Hz)           │  │  (10 Hz)            │
│                     │  │                     │
│  while (running):   │  │  while (running):   │
│    callback()       │  │    callback()       │
│    sleep_until(next)│  │    sleep_until(next)│
└─────────────────────┘  └─────────────────────┘
```

Each subscription and loop runs in its own thread. Signal handlers set a flag that all threads check.

## Data Flow Example

Publishing an IMU reading (48 bytes):

```
1. Publisher calls: pub.publish(&imu_data, 48)

2. RingBufferWriter::try_write():
   - Check: 48 + 20 = 68 bytes < 4096 slot size ✓
   - slot_index = write_idx(1000) % 16 = 8
   - Write to slot 8:
     - bytes 0-3:   size = 48
     - bytes 4-11:  sequence = 1000
     - bytes 12-19: timestamp = 9876543210
     - bytes 20-67: imu_data
   - write_idx.store(1001, release)
   - futex_word++ and wake_all()

3. Subscriber wakes from futex_wait()

4. RingBufferReader::try_read():
   - read_idx(1000) < write_idx(1001) → data available
   - slot_index = 1000 % 16 = 8
   - Read header: size=48, sequence=1000
   - Verify: 1000 == 1000 ✓
   - read_idx.store(1001)
   - Return pointer to bytes 20-67

5. Subscriber callback receives pointer directly into shared memory
   - NO COPY occurred
```

## Memory Sizing

Total shared memory per topic:
```
size = sizeof(Header) + slot_count × slot_size
     ≈ 1152 + 16 × 4096
     ≈ 66 KB (default)
```

For large messages (e.g., 1 MB images):
```cpp
RingBufferConfig config;
config.slot_count = 4;
config.slot_size = 1'100'000;  // 1 MB + header

Publisher pub("camera", config);
// size ≈ 1152 + 4 × 1.1 MB ≈ 4.4 MB
```

## Performance Characteristics

| Operation | Typical Latency | Notes |
|-----------|-----------------|-------|
| `publish()` small | 200-500 ns | Dominated by memcpy + futex wake |
| `publish()` 1 MB | 100-200 µs | memcpy bound |
| `wait()` data ready | 100-200 ns | Just pointer arithmetic |
| `wait()` from sleep | 2-5 µs | futex kernel overhead |
| `get_timestamp_ns()` | 20-30 ns | vDSO optimized |

## File Locations

| File | Purpose |
|------|---------|
| `src/internal/shm_region.cpp` | Shared memory create/open |
| `src/internal/ring_buffer.cpp` | Lock-free ring buffer |
| `src/internal/futex.cpp` | Sleep/wake signaling |
| `src/internal/time.cpp` | Nanosecond timestamps |
| `src/pubsub.cpp` | Publisher/Subscriber classes |
| `src/node.cpp` | Node class with threading |
