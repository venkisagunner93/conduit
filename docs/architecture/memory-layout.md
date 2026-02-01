# Memory Layout

Here's what the shared memory actually looks like:

## Full structure

```
/dev/shm/conduit_imu (with 16 slots, 4KB each)
┌─────────────────────────────────────────────────────────────────┐
│                        HEADER (~1 KB)                           │
│                                                                 │
│  Configuration (set once by publisher):                         │
│    slot_count = 16                                              │
│    slot_size = 4096                                             │
│    max_subscribers = 16                                         │
│                                                                 │
│  Coordination (updated during operation):                       │
│    write_idx = 1000        ← Publisher's counter                │
│    futex_word = 1000       ← Doorbell counter                   │
│    subscriber_mask = 0b111 ← 3 subscribers active (bits 0,1,2)  │
│    read_idx[0] = 1000      ← Subscriber 0's counter             │
│    read_idx[1] = 998       ← Subscriber 1's counter             │
│    read_idx[2] = 995       ← Subscriber 2's counter             │
│    read_idx[3..15] = 0     ← Unused                             │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│  Slot 0: [size][sequence][timestamp][payload................]   │
├─────────────────────────────────────────────────────────────────┤
│  Slot 1: [size][sequence][timestamp][payload................]   │
├─────────────────────────────────────────────────────────────────┤
│  ...                                                            │
├─────────────────────────────────────────────────────────────────┤
│  Slot 15: [size][sequence][timestamp][payload...............]   │
└─────────────────────────────────────────────────────────────────┘
```

## Header details

```
Byte offset   Field                    Size        Notes
────────────────────────────────────────────────────────────────────
0             slot_count               4 bytes     Power of 2 (e.g., 16)
4             slot_size                4 bytes     Bytes per slot
8             max_subscribers          4 bytes     Always 16
12            (padding)                4 bytes

16            write_idx                8 bytes     Atomic counter
24            subscriber_mask          4 bytes     Bitmap of active subscribers
28            (padding)                4 bytes
32            futex_word               4 bytes     Doorbell counter
36            (padding)                28 bytes    Align to cache line

64            read_idx[0]              64 bytes    Cache-line aligned
128           read_idx[1]              64 bytes
...
1024          read_idx[15]             64 bytes
────────────────────────────────────────────────────────────────────
              Total header: ~1088 bytes
```

## Why cache-line alignment?

Each `read_idx` is on its own 64-byte cache line.

Without alignment:
```
CPU 0 updates read_idx[0]
CPU 1 updates read_idx[1]
→ Both values are on the same cache line
→ CPUs fight over the cache line ("false sharing")
→ Performance tanks
```

With alignment:
```
CPU 0 updates read_idx[0] on cache line A
CPU 1 updates read_idx[1] on cache line B
→ No interference
→ Full speed
```

## Slot details

Each slot is `slot_size` bytes:

```
Byte offset   Field                    Size
────────────────────────────────────────────────────────────────────
0             size                     4 bytes     Payload size
4             sequence                 8 bytes     Message number
12            timestamp_ns             8 bytes     When published
20            payload                  (slot_size - 20) bytes
```

## Total memory per topic

```
total = header_size + (slot_count × slot_size)
      = 1088 + (16 × 4096)
      = 1088 + 65536
      = 66,624 bytes (~65 KB)
```

For large messages (1 MB images, 4 slots):
```
total = 1088 + (4 × 1,100,000)
      = 1088 + 4,400,000
      = ~4.4 MB
```

---

**Next:** [Complete Flow](complete-flow.md) — End-to-end message trace
