# Ring Buffer

Shared memory is just raw bytes. We need structure. Enter the ring buffer.

## The Problem

Publisher sends messages continuously. Where do they go?

**Option 1:** One slot for the latest message
```
[message]
```
Problem: If publisher writes while subscriber reads, data corrupts.

**Option 2:** Infinite queue
```
[msg1][msg2][msg3][msg4]...forever
```
Problem: Memory grows forever.

**Option 3: Ring buffer** — fixed slots that wrap around
```
[slot 0][slot 1][slot 2][slot 3]
    ↑                       ↑
    └── after slot 3, ──────┘
        wrap to slot 0
```

## How it works

Think of 4 mailboxes in a row. The mail carrier (publisher) puts letters in them one by one:

```
Step 1: Put letter in box 0
[LETTER][     ][     ][     ]
   ↑

Step 2: Put letter in box 1
[letter][LETTER][     ][     ]
            ↑

Step 3: Put letter in box 2
[letter][letter][LETTER][     ]
                   ↑

Step 4: Put letter in box 3
[letter][letter][letter][LETTER]
                           ↑

Step 5: Wrap around! Put letter in box 0 (overwrites old)
[LETTER][letter][letter][letter]
   ↑
```

The ring buffer does exactly this. With 4 slots:
- Message 0 → slot 0
- Message 1 → slot 1
- Message 2 → slot 2
- Message 3 → slot 3
- Message 4 → slot 0 (wraps, overwrites message 0)
- Message 5 → slot 1 (wraps, overwrites message 1)
- ...

**Formula:** `slot = message_number % slot_count`

## What's in each slot?

Each slot is a contiguous block of bytes. The first 20 bytes are a header (bookkeeping), followed by your actual data.

```
One Slot (e.g., 4096 bytes total)
┌────────────────────────────────────────────────────────────────┐
│ HEADER (20 bytes)                  │ PAYLOAD (up to 4076 bytes)│
├────────────────────────────────────┼───────────────────────────┤
│ size     │ sequence  │ timestamp   │ your data                 │
│ (4 bytes)│ (8 bytes) │ (8 bytes)   │                           │
└────────────────────────────────────┴───────────────────────────┘
 byte 0-3   byte 4-11   byte 12-19    byte 20 onwards
```

**Header fields:**
- `size` — How many bytes of payload (so subscriber knows where data ends)
- `sequence` — Which message number this is (for overwrite detection)
- `timestamp` — When it was published (for latency measurement)

## Concrete example: IMU message

Let's say you have this IMU struct:

```cpp
struct ImuData {
    float accel_x;   // 4 bytes
    float accel_y;   // 4 bytes
    float accel_z;   // 4 bytes
    float gyro_x;    // 4 bytes
    float gyro_y;    // 4 bytes
    float gyro_z;    // 4 bytes
};                   // Total: 24 bytes
```

With values: `accel = (0.1, 0.2, 9.8)`, `gyro = (0.01, 0.02, 0.03)`

When published as message #1000, the slot looks like this:

```
Byte offset   Contents                          Meaning
─────────────────────────────────────────────────────────────────
0-3           18 00 00 00                       size = 24 (payload bytes)
4-11          E8 03 00 00 00 00 00 00           sequence = 1000
12-19         15 CD 5B 07 00 00 00 00           timestamp = 123456789 ns
─────────────────────────────────────────────────────────────────
20-23         CD CC CC 3D                       accel_x = 0.1f
24-27         CD CC 4C 3E                       accel_y = 0.2f
28-31         CD CC 1C 41                       accel_z = 9.8f
32-35         0A D7 23 3C                       gyro_x = 0.01f
36-39         0A D7 A3 3C                       gyro_y = 0.02f
40-43         0A D7 F3 3C                       gyro_z = 0.03f
─────────────────────────────────────────────────────────────────
44-4095       (unused space in this slot)
```

**What subscriber gets:**
- Pointer to byte 20 (start of your IMU data)
- Size = 24 (from header)
- Sequence = 1000 (from header)
- Timestamp (from header)

The subscriber casts the pointer: `ImuData* imu = (ImuData*)msg.data();`

---

**Next:** [Indices](indices.md) — How publisher and subscriber coordinate
