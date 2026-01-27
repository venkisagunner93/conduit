# Architecture

Conduit is a layered system built from simple primitives.

## Layer Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│                                                             │
│   Node - Thread management, signal handling, callbacks      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Transport Layer                          │
│                                                             │
│   Publisher / Subscriber - High-level pub/sub API           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     Buffer Layer                            │
│                                                             │
│   RingBuffer - Lock-free SPMC queue                         │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     Memory Layer                            │
│                                                             │
│   ShmRegion - Shared memory management                      │
│   Futex - Sleep/wake signaling                              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Operating System                         │
│                                                             │
│   /dev/shm - RAM-backed filesystem                          │
│   futex() - Kernel wait/wake syscall                        │
└─────────────────────────────────────────────────────────────┘
```

## Package Structure

| Package | Description |
|---------|-------------|
| `conduit_core` | Ring buffer, shared memory, futex, pub/sub, node, logging |
| `conduit_tank` | MCAP-based recording with Zstd/LZ4 compression |
| `conduit_flow` | Flow execution engine for startup/shutdown sequences |
| `conduit_demo` | Example publisher and subscriber applications |
| `conduit_tools` | CLI tools (topics, info, echo, hz, record, flow) |

## Design Principles

### Zero-Copy

Traditional pub/sub copies data multiple times:

```
Publisher → Kernel → Network → Kernel → Subscriber
              ↑                   ↑
           copy #1             copy #2
```

Conduit uses shared memory:

```
Publisher → Shared Memory ← Subscriber
               ↑
           one write, zero copies to read
```

### Lock-Free

No mutexes means:

- No deadlocks
- Crashed processes don't block others
- Better performance under contention

Coordination uses atomic operations and sequence numbers.

### Zero CPU When Idle

Subscribers use futex to sleep efficiently:

- Not spinning (0% CPU)
- Not polling (no latency penalty)
- Instant wake when data arrives

## Data Flow

### Publishing

1. Write message to ring buffer slot
2. Update write index (atomic)
3. Wake sleeping subscribers (futex)

### Subscribing

1. Check if new data available (compare indices)
2. If no data, sleep on futex
3. When woken, read from slot
4. Verify sequence number (detect overwrites)
5. Return pointer to shared memory (zero-copy)

## Topic Lifecycle

```
Publisher creates topic:
  shm_open("/conduit_topic", O_CREAT)
  ftruncate(size)
  mmap()
  Initialize ring buffer header

Subscriber joins:
  shm_open("/conduit_topic", O_RDWR)
  mmap() → same physical memory!
  claim_slot() → reserve read index

Communication:
  Publisher writes to slots
  Subscribers read from slots
  All see same memory

Cleanup:
  Subscribers release slots
  Publisher calls shm_unlink()
```
