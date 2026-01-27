# Internals

This page documents the low-level implementation details of Conduit.

## Source Files

The core implementation lives in `packages/conduit_core/src/internal/`:

| File | Purpose |
|------|---------|
| `shm_region.cpp` | Shared memory create/open/unlink |
| `ring_buffer.cpp` | Lock-free circular buffer |
| `futex.cpp` | Sleep/wake signaling |
| `time.cpp` | Nanosecond timestamps |

## Data Flow Diagram

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
                    │                              │
          ┌────────┴────────┐            ┌────────┴────────┐
          ▼                 ▼            ▼                 ▼
┌─────────────────┐  ┌─────────────┐  ┌──────────────┐  ┌─────────────┐
│   SHM REGION    │  │    FUTEX    │  │  SHM REGION  │  │    FUTEX    │
│ (shm_region.cpp)│  │ (futex.cpp) │  │              │  │             │
│                 │  │             │  │              │  │             │
│ create/open     │  │ wake_all()  │  │ open()       │  │ wait()      │
│ shared memory   │  │             │  │              │  │             │
└─────────────────┘  └─────────────┘  └──────────────┘  └─────────────┘
                           │                    │
                           └────────┬───────────┘
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                      /dev/shm/conduit_imu (SHARED MEMORY)                   │
│                                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                         RING BUFFER HEADER                          │   │
│   │  write_idx=42  |  read_idx[0]=40  |  futex_word=42  | ...           │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│   ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐              │
│   │ Slot 0  │ │ Slot 1  │ │ Slot 2  │ │ Slot 3  │ │  ...    │              │
│   └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Publishing an Integer (Step by Step)

### 1. Publisher Creates Topic

```cpp
Publisher pub("counter");
```

**shm_region.cpp:**
```
shm_open("/conduit_counter", O_CREAT | O_EXCL | O_RDWR)  → creates file
ftruncate(fd, 69632)                                      → allocates RAM
mmap(...)                                                 → maps to address space
memset(ptr, 0, size)                                      → zeros memory
```

### 2. Initialize Ring Buffer

**ring_buffer.cpp:**
```
header->slot_count = 16
header->slot_size = 4096
header->write_idx = 0
header->read_idx[0..15] = 0
header->futex_word = 0
```

### 3. Subscriber Opens Topic

```cpp
Subscriber sub("counter");
```

**shm_region.cpp:**
```
shm_open("/conduit_counter", O_RDWR)  → opens existing file
fstat(fd)                              → gets size
mmap(...)                              → maps SAME memory!
```

**ring_buffer.cpp:**
```
claim_slot()                           → atomically claims slot 0
read_idx[0] = write_idx                → start at current position
```

### 4. Publisher Writes Message

```cpp
int value = 42;
pub.publish(&value, 4);
```

**ring_buffer.cpp try_write():**
```
1. Check: 4 + 20 header = 24 bytes < 4096 slot ✓
2. slot_index = write_idx(0) % 16 = 0
3. Write to slot 0:
   - offset 0:  size = 4
   - offset 4:  sequence = 0
   - offset 12: timestamp = 1234567890
   - offset 20: value = 42
4. write_idx.store(1, release)
5. futex_word.fetch_add(1)
6. futex_wake_all()
```

### 5. Subscriber Reads Message

```cpp
Message msg = sub.wait();
```

**ring_buffer.cpp wait():**
```
1. read_idx[0]=0, write_idx=1 → data available!
2. slot_index = 0 % 16 = 0
3. Read slot 0: size=4, sequence=0, timestamp=...
4. Verify: sequence(0) == read_idx(0) ✓
5. read_idx[0].store(1)
6. Return pointer to offset 20
```

**Zero copy:** Subscriber gets pointer directly into shared memory.

## Memory Layout

### Ring Buffer Header

```
Offset    Field                Size
───────────────────────────────────
0         slot_count           4
4         slot_size            4
8         max_subscribers      4
12        padding              4
16        write_idx            8
24        subscriber_mask      4
28        padding              4
32        futex_word           4
36        padding              28
64        read_idx[0]          64 (cache-line aligned)
128       read_idx[1]          64
...
1088      read_idx[15]         64
───────────────────────────────────
Total: ~1152 bytes
```

### Slot Layout

```
Offset    Field                Size
───────────────────────────────────
0         size                 4
4         sequence             8
12        timestamp_ns         8
20        payload              (slot_size - 20)
```

## Performance Characteristics

| Operation | Typical Latency |
|-----------|-----------------|
| `publish()` small message | 200-500 ns |
| `publish()` 1MB message | 100-200 µs |
| `wait()` when data ready | 100-200 ns |
| `wait()` wake from sleep | 2-5 µs |
| `get_timestamp_ns()` | 20-30 ns |

## Further Reading

For detailed explanations with commented source code, see:
`packages/conduit_core/src/internal/README.md`
