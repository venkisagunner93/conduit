# Complete Flow

Let's trace a message from publisher to subscriber, step by step.

## Publisher sends IMU data

```cpp
ImuData imu = {.accel = {0.1, 0.2, 9.8}, .gyro = {0, 0, 0.01}};
pub.publish(&imu, sizeof(imu));  // 24 bytes
```

### Step 1: Check size

```
24 bytes + 20 byte header = 44 bytes
44 < 4096 slot size ✓
```

### Step 2: Find slot

```
write_idx = 1000
slot = 1000 % 16 = 8
```

### Step 3: Write to slot 8

```
Slot 8:
  bytes 0-3:   size = 24
  bytes 4-11:  sequence = 1000
  bytes 12-19: timestamp = [current time]
  bytes 20-43: [24 bytes of IMU data]
```

### Step 4: Update write_idx

```
write_idx = 1001  (atomic store with release ordering)
```

This ensures all the slot data is visible before other threads see the new write_idx.

### Step 5: Ring doorbell

```
futex_word++ → 1001
futex_wake_all()  → wake sleeping subscribers
```

## Subscriber receives

### Step 1: Check for data

```
read_idx = 1000
write_idx = 1001
1001 > 1000 → new data available!
```

### Step 2: Find slot

```
slot = 1000 % 16 = 8
```

### Step 3: Verify not overwritten

```
slot 8 header: sequence = 1000
expected: 1000
1000 == 1000 ✓ Data is valid
```

### Step 4: Return pointer

```
Return pointer to bytes 20-43 of slot 8
(This points directly into shared memory — zero copy!)
```

### Step 5: Update read_idx

```
read_idx = 1001
```

## Full timeline

```
Time    Publisher                          Subscriber
────────────────────────────────────────────────────────────────────

T0      write_idx = 1000                   read_idx = 1000
        (no new data)                      Sleeping on futex (CPU = 0%)

T1      pub.publish(&imu, 24)
        └─ Find slot: 1000 % 16 = 8
        └─ Write header + payload to slot 8

T2      └─ write_idx = 1001 (atomic)
        └─ futex_word = 1001
        └─ futex_wake_all()
                                           ← Kernel wakes subscriber

T3                                         Woke up!
                                           Check: 1001 > 1000 → data!
                                           Read slot 8
                                           Verify: seq 1000 == 1000 ✓
                                           read_idx = 1001

T4                                         callback(msg)
                                           └─ msg.data() → slot 8 byte 20
                                           └─ msg.size() → 24
                                           └─ Process IMU data...

T5                                         Check: 1001 > 1001? NO
                                           Sleep on futex again (CPU = 0%)
```

## What makes this fast

1. **No data copying** — Subscriber reads directly from shared memory
2. **No serialization** — Just raw bytes, interpreted by the receiver
3. **No kernel calls** (except futex) — All coordination via atomic memory operations
4. **No locks** — Lock-free ring buffer, no mutex contention

## Latency breakdown

| Operation | Time |
|-----------|------|
| Find slot (modulo) | ~1 ns |
| Write header | ~10 ns |
| Write payload (24 bytes) | ~10 ns |
| Atomic write_idx update | ~10 ns |
| Futex wake (kernel call) | ~1-2 µs |
| **Total publish** | **~2 µs** |
| Futex wake latency | ~2-5 µs |
| Read slot header | ~10 ns |
| Return pointer | ~1 ns |
| **Total end-to-end** | **~5-10 µs** |

For a 24-byte IMU message, you get single-digit microsecond latency.

---

**Back to:** [Architecture Overview](index.md)
