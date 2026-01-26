# Conduit Core Internals

This directory contains the low-level building blocks for Conduit's zero-copy pub/sub system. If you're new to Linux shared memory or lock-free programming, start here.

## Quick Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           YOUR APPLICATION                                  │
│                                                                             │
│   Publisher pub("imu");              Subscriber sub("imu");                 │
│   pub.publish(data, size);           sub.wait() -> Message                  │
└─────────────────────────────────────────────────────────────────────────────┘
                    │                              │
                    ▼                              ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         RING BUFFER (ring_buffer.cpp)                       │
│                                                                             │
│   RingBufferWriter::try_write()      RingBufferReader::wait()               │
│   - Writes to slot                   - Reads from slot                      │
│   - Wakes subscribers                - Sleeps if no data                    │
└─────────────────────────────────────────────────────────────────────────────┘
                   │                                  │
          ┌────────┴────────┐                ┌────────┴───────┐
          ▼                 ▼                ▼                ▼
┌─────────────────┐  ┌─────────────┐  ┌──────────────┐ ┌─────────────┐
│   SHM REGION    │  │    FUTEX    │  │   SHM REGION │ │    FUTEX    │
│ (shm_region.cpp)│  │ (futex.cpp) │  │              │ │             │
│                 │  │             │  │              │ │             │
│ create/open     │  │ wake_all()  │  │ open()       │ │ wait()      │
│ shared memory   │  │             │  │              │ │             │
└─────────────────┘  └─────────────┘  └──────────────┘ └─────────────┘
          │                                    │
          └────────────────┬───────────────────┘
                           ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                      /dev/shm/conduit_imu (SHARED MEMORY)                   │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                         RING BUFFER HEADER                          │   │
│   │  write_idx=42  |  read_idx[0]=40  |  futex_word=42  | ...           │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐               │
│   │ Slot 0  │ │ Slot 1  │ │ Slot 2  │ │ Slot 3  │ │  ...    │               │
│   │ [data]  │ │ [data]  │ │ [data]  │ │ [data]  │ │         │               │
│   └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘               │
└─────────────────────────────────────────────────────────────────────────────┘
```

## The Files

| File | Purpose | Key Concept |
|------|---------|-------------|
| `shm_region.cpp` | Create/open shared memory | Memory visible to multiple processes |
| `ring_buffer.cpp` | Circular message queue | Lock-free single-producer, multi-consumer |
| `futex.cpp` | Sleep/wake signaling | Zero CPU when waiting |
| `time.cpp` | Nanosecond timestamps | Monotonic clock for latency measurement |

---

## Data Flow: Publishing an `int`

Let's trace how a simple integer (4 bytes) flows from publisher to subscriber.

### Step 1: Publisher Creates Topic

```cpp
Publisher pub("counter");  // Creates /dev/shm/conduit_counter
```

**What happens (shm_region.cpp):**
1. `shm_open("/conduit_counter", O_CREAT | O_EXCL | O_RDWR)` - creates file
2. `ftruncate(fd, size)` - allocates RAM
3. `mmap()` - maps RAM into publisher's address space
4. Returns pointer to shared memory

**Result:** Publisher has a pointer to ~70KB of RAM that will be shared.

### Step 2: Publisher Initializes Ring Buffer

```cpp
// Inside Publisher constructor
RingBufferWriter writer(shm_ptr, size, config);
writer.initialize();
```

**What happens (ring_buffer.cpp):**
1. Casts `shm_ptr` to `RingBufferHeader*`
2. Sets `slot_count=16`, `slot_size=4096`, etc.
3. Sets `write_idx=0`, all `read_idx[]=0`
4. Memory fence ensures all writes are visible

**Memory layout after init:**
```
[Header: write_idx=0, read_idx[0..15]=0, futex_word=0]
[Slot 0: empty][Slot 1: empty]...[Slot 15: empty]
```

### Step 3: Subscriber Opens Topic

```cpp
Subscriber sub("counter");  // Opens /dev/shm/conduit_counter
```

**What happens (shm_region.cpp):**
1. `shm_open("/conduit_counter", O_RDWR)` - opens existing file
2. `fstat()` - gets size
3. `mmap()` - maps into subscriber's address space
4. Now subscriber sees the SAME memory!

**What happens (ring_buffer.cpp):**
1. `claim_slot()` - atomically claims slot 0 for this subscriber
2. Sets `read_idx[0] = write_idx` (starts at current position)

### Step 4: Publisher Writes Message

```cpp
int value = 42;
pub.publish(&value, sizeof(value));
```

**What happens (ring_buffer.cpp `try_write`):**

```
1. Check size: 4 bytes + 20 byte header = 24 bytes < 4096 slot size ✓

2. Calculate slot: write_idx(0) % 16 = slot 0

3. Write to slot 0:
   Offset 0-3:   size = 4
   Offset 4-11:  sequence = 0
   Offset 12-19: timestamp = 1234567890123
   Offset 20-23: value = 42  <-- THE ACTUAL DATA

4. Update write_idx: 0 → 1 (with release memory ordering)

5. Wake subscribers:
   - Increment futex_word: 0 → 1
   - futex_wake_all() - wakes any sleeping subscribers
```

**Memory after write:**
```
[Header: write_idx=1, read_idx[0]=0, futex_word=1]
[Slot 0: size=4, seq=0, ts=..., data=42][Slot 1: empty]...
```

### Step 5: Subscriber Reads Message

```cpp
Message msg = sub.wait();
int value = *(int*)msg.data();  // value = 42!
```

**What happens (ring_buffer.cpp `try_read`):**

```
1. Load indices: read_idx[0]=0, write_idx=1
   0 < 1 → data available!

2. Calculate slot: read_idx(0) % 16 = slot 0

3. Read slot 0 header:
   size = 4
   sequence = 0
   timestamp = 1234567890123

4. Verify sequence: 0 == 0 ✓ (data wasn't overwritten)

5. Return pointer to offset 20 (the payload)

6. Update read_idx[0]: 0 → 1
```

**The subscriber gets a pointer DIRECTLY into shared memory!**
No copy of the data occurred. Both processes see the same RAM.

---

## Data Flow: Publishing a PointCloud (12MB)

Now let's see how a large message works.

### Setup

```cpp
// Configure for large messages
RingBufferConfig config;
config.slot_count = 4;           // 4 slots
config.slot_size = 13'000'000;   // 13MB per slot (12MB payload + header)

// Total shared memory: ~52MB
Publisher pub("lidar", config);
```

### Publishing

```cpp
struct PointCloud {
    float x[1000000];  // 4MB
    float y[1000000];  // 4MB
    float z[1000000];  // 4MB
};  // Total: 12MB

PointCloud cloud;
// ... fill cloud with data ...

pub.publish(&cloud, sizeof(cloud));  // Writes 12MB
```

**What happens:**
1. `memcpy(slot_ptr + 20, &cloud, 12'000'000)` - copies into shared memory
2. This is the ONLY copy!

### Subscribing

```cpp
Message msg = sub.wait();
PointCloud* cloud = (PointCloud*)msg.data();

// Access the data directly - NO COPY
float first_x = cloud->x[0];
float first_y = cloud->y[0];
```

**The subscriber reads the 12MB directly from shared memory.**

Traditional pub/sub with TCP/IP would copy:
1. User → kernel send buffer
2. Network stack processing
3. Kernel receive buffer → user

That's 3+ copies. Conduit does ONE copy (into shared memory), and subscribers read directly.

---

## Key Concepts Explained

### Shared Memory (/dev/shm)

On Linux, `/dev/shm` is a RAM-based filesystem. Files there are stored in memory, not on disk.

```bash
# After running publisher:
$ ls -la /dev/shm/conduit_*
-rw-rw-rw- 1 user user 69632 Jan 15 10:30 /dev/shm/conduit_counter
```

When two processes `mmap()` the same file, they see the same physical RAM. Changes by one are immediately visible to the other.

### Lock-Free Ring Buffer

"Lock-free" means no mutexes. Coordination happens through:

1. **Atomic operations**: `atomic<uint64_t>` for indices
2. **Memory ordering**: `memory_order_release` / `memory_order_acquire`
3. **Sequence numbers**: Detect if data was overwritten

Benefits:
- No deadlocks possible
- Crashed process doesn't block others
- Better performance (no lock contention)

### Futex (Fast Userspace Mutex)

Problem: How does subscriber sleep efficiently?

**Bad: Busy-wait**
```cpp
while (!has_data()) { }  // Burns 100% CPU
```

**Bad: Sleep polling**
```cpp
while (!has_data()) { sleep(10ms); }  // Adds latency
```

**Good: Futex**
```cpp
while (!has_data()) {
    futex_wait(&word, expected);  // Kernel puts us to sleep
}
```

With futex:
- Zero CPU when no data (truly asleep in kernel)
- Instant wake when data arrives (kernel wakes us directly)

### Memory Ordering

CPUs reorder operations for performance. We need to control this:

```cpp
// Publisher
memcpy(slot, data, size);                           // Step 1
write_idx.store(idx + 1, memory_order_release);     // Step 2
```

`memory_order_release` ensures Step 1 completes before Step 2 becomes visible to other CPUs.

```cpp
// Subscriber
uint64_t write_idx = header->write_idx.load(memory_order_acquire);  // Step A
memcpy(out, slot, size);                                             // Step B
```

`memory_order_acquire` ensures Step B sees all writes that happened before the publisher's release.

---

## Function Reference

### shm_region.cpp

| Function | Description |
|----------|-------------|
| `ShmRegion::create(name, size)` | Create new shared memory (publisher) |
| `ShmRegion::open(name)` | Open existing shared memory (subscriber) |
| `ShmRegion::exists(name)` | Check if topic exists |
| `ShmRegion::wait_until_exists(name, running)` | Poll until topic appears |
| `ShmRegion::unlink(name)` | Delete shared memory file |

### ring_buffer.cpp

**Writer (Publisher):**
| Function | Description |
|----------|-------------|
| `RingBufferWriter(region, size, config)` | Create writer over shared memory |
| `initialize()` | Set up header (call once) |
| `try_write(data, len)` | Write message, returns false if too large |

**Reader (Subscriber):**
| Function | Description |
|----------|-------------|
| `RingBufferReader(region, size)` | Create reader over shared memory |
| `claim_slot()` | Get subscriber slot (0-15), -1 if full |
| `release_slot(slot)` | Free subscriber slot |
| `try_read(slot)` | Non-blocking read, returns nullopt if empty |
| `wait(slot)` | Blocking read, waits forever |
| `wait_for(slot, timeout)` | Blocking read with timeout |

### futex.cpp

| Function | Description |
|----------|-------------|
| `futex_wait(word, expected, timeout)` | Sleep until word changes |
| `futex_wake(word, count)` | Wake up to N waiters |
| `futex_wake_all(word)` | Wake all waiters |

### time.cpp

| Function | Description |
|----------|-------------|
| `get_timestamp_ns()` | Nanoseconds since boot (CLOCK_MONOTONIC_RAW) |

---

## Performance Characteristics

| Operation | Typical Latency |
|-----------|-----------------|
| `publish()` small message | ~200-500 ns |
| `publish()` 1MB message | ~100-200 µs (memcpy bound) |
| `wait()` when data ready | ~100-200 ns |
| `wait()` wake from sleep | ~2-5 µs (futex overhead) |
| `get_timestamp_ns()` | ~20-30 ns |

---

## Common Questions

**Q: What if publisher crashes?**
A: Subscribers can continue reading existing data. The shared memory persists until explicitly unlinked or system reboot.

**Q: What if subscriber is too slow?**
A: Publisher overwrites old slots. Subscriber detects this via sequence number mismatch and skips to newest data.

**Q: Maximum message size?**
A: Configured at topic creation. Default is 4KB per slot. For large messages (images, point clouds), increase `slot_size`.

**Q: Maximum subscribers per topic?**
A: 16 (hardcoded as `MAX_SUBSCRIBERS`). Each needs a read index slot.

**Q: Why power-of-2 slot count?**
A: Enables fast modulo via bitmask: `idx % 16` becomes `idx & 15`.
