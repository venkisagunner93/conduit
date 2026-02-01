# Architecture

This page explains how Conduit works, piece by piece.

## The Core Idea

Imagine a whiteboard in a shared office. Anyone in the office can read it. One person writes on it.

That's Conduit.

- **The whiteboard** = shared memory (a chunk of RAM both processes can access)
- **The writer** = publisher
- **The readers** = subscribers

No photocopying. No passing notes. Everyone looks at the same whiteboard.

---

## Part 1: Shared Memory

### What is it?

Normally, each program has its own private memory. Program A can't see Program B's variables.

Shared memory breaks this rule. It's a region of RAM that multiple programs can access:

```
Program A's memory:     [private stuff] [SHARED] [more private]
                                           ↓
                                    Same physical RAM
                                           ↑
Program B's memory:     [private stuff] [SHARED] [more private]
```

On Linux, shared memory appears as files in `/dev/shm/`:

```bash
$ ls /dev/shm/
conduit_imu      # Topic "imu"
conduit_camera   # Topic "camera"
```

These aren't real files on disk. They're chunks of RAM with names.

### How Conduit uses it

When you create a publisher:

```cpp
Publisher pub("imu");
```

Conduit creates `/dev/shm/conduit_imu` — a chunk of shared memory.

When you create a subscriber:

```cpp
Subscriber sub("imu");
```

Conduit opens that same chunk. Now both processes see the same memory.

**That's the zero-copy magic.** Publisher writes bytes. Subscriber reads those exact bytes. No copying between processes.

---

## Part 2: The Ring Buffer

Shared memory is just raw bytes. We need structure. Enter the ring buffer.

### The Problem

Publisher sends messages continuously. Where do they go?

Option 1: One slot for the latest message
```
[message]
```
Problem: If publisher writes while subscriber reads, data corrupts.

Option 2: Infinite queue
```
[msg1][msg2][msg3][msg4]...forever
```
Problem: Memory grows forever.

Option 3: **Ring buffer** — fixed slots that wrap around
```
[slot 0][slot 1][slot 2][slot 3]
    ↑                       ↑
    └── after slot 3, ──────┘
        wrap to slot 0
```

### How it works

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

### What's in each slot?

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

### Concrete example: IMU message

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

## Part 3: Write Index and Read Index

Now the key question: how do publisher and subscriber coordinate?

### The write index

The publisher keeps a counter: "I've written N messages total."

Let's watch the shared memory for topic "imu" as messages are published:

```
INITIAL STATE (publisher just created topic)
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 0           ← "No messages written yet"           │
│   read_idx[0] = 0                                               │
│   read_idx[1..15] = 0                                           │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [empty]                                                 │
│ Slot 1: [empty]                                                 │
│ Slot 2: [empty]                                                 │
│ Slot 3: [empty]                                                 │
└─────────────────────────────────────────────────────────────────┘
```

Publisher writes first IMU message:

```
AFTER FIRST PUBLISH
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 1           ← "1 message written"                 │
│   read_idx[0] = 0                                               │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [size=24][seq=0][ts=1000][accel=0.1,0.2,9.8 gyro=...]   │ ← msg 0
│ Slot 1: [empty]                                                 │
│ Slot 2: [empty]                                                 │
│ Slot 3: [empty]                                                 │
└─────────────────────────────────────────────────────────────────┘
```

Publisher writes more messages:

```
AFTER 5 PUBLISHES
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 5           ← "5 messages written"                │
│   read_idx[0] = 0                                               │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [size=24][seq=4][ts=1400][accel=0.1,0.2,9.7 gyro=...]   │ ← msg 4 (overwrote msg 0)
│ Slot 1: [size=24][seq=1][ts=1100][accel=0.1,0.2,9.8 gyro=...]   │ ← msg 1
│ Slot 2: [size=24][seq=2][ts=1200][accel=0.1,0.2,9.8 gyro=...]   │ ← msg 2
│ Slot 3: [size=24][seq=3][ts=1300][accel=0.1,0.2,9.7 gyro=...]   │ ← msg 3
└─────────────────────────────────────────────────────────────────┘

Note: msg 4 went to slot 0 (4 % 4 = 0), overwriting msg 0
```

### The read index

Now a subscriber joins. Each subscriber gets its own read_idx slot.

```
SUBSCRIBER JOINS
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 5                                                 │
│   read_idx[0] = 5         ← Subscriber 0: "I'll start here"     │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [size=24][seq=4][ts=1400][...]                          │ ← msg 4
│ Slot 1: [size=24][seq=1][ts=1100][...]                          │ ← msg 1 (old)
│ Slot 2: [size=24][seq=2][ts=1200][...]                          │ ← msg 2 (old)
│ Slot 3: [size=24][seq=3][ts=1300][...]                          │ ← msg 3 (old)
└─────────────────────────────────────────────────────────────────┘

Subscriber checks: write_idx (5) > read_idx (5)? NO → no new data, wait...
```

Publisher writes message 5:

```
PUBLISHER WRITES, SUBSCRIBER WAITING
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 6           ← Publisher updated this              │
│   read_idx[0] = 5         ← Subscriber still at 5               │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [size=24][seq=4][ts=1400][...]                          │ ← msg 4
│ Slot 1: [size=24][seq=5][ts=1500][accel=0.1,0.2,9.8 gyro=...]   │ ← msg 5 (NEW!)
│ Slot 2: [size=24][seq=2][ts=1200][...]                          │ ← msg 2 (old)
│ Slot 3: [size=24][seq=3][ts=1300][...]                          │ ← msg 3 (old)
└─────────────────────────────────────────────────────────────────┘

Subscriber checks: write_idx (6) > read_idx (5)? YES → new data!
Subscriber: "I want message 5. Slot = 5 % 4 = 1. Read slot 1."
```

Subscriber reads and updates:

```
SUBSCRIBER READS MESSAGE
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 6                                                 │
│   read_idx[0] = 6         ← Subscriber updated: "I read msg 5"  │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [size=24][seq=4][ts=1400][...]                          │
│ Slot 1: [size=24][seq=5][ts=1500][...]  ← Subscriber read this  │
│ Slot 2: [size=24][seq=2][ts=1200][...]                          │
│ Slot 3: [size=24][seq=3][ts=1300][...]                          │
└─────────────────────────────────────────────────────────────────┘

Subscriber checks: write_idx (6) > read_idx (6)? NO → caught up, wait...
```

### Finding the right slot

Which slot has message #7?

```
slot = message_number % slot_count
slot = 7 % 4 = 3

So message #7 is in slot 3.
```

### Putting it together

```
Ring buffer with 4 slots:

write_idx = 7 (publisher has written messages 0-6)

[slot 0]  [slot 1]  [slot 2]  [slot 3]
 msg 4     msg 5     msg 6     msg 7
                               ↑
                         latest message

Subscriber A: read_idx = 7 (caught up, waiting)
Subscriber B: read_idx = 5 (2 messages behind, reading msg 5)
```

---

## Part 4: The Sequence Number (Overwrite Detection)

### The problem

What if a subscriber is too slow?

```
4 slots, subscriber is way behind:

write_idx = 100
read_idx = 90   ← Subscriber wants message 90

But slot = 90 % 4 = slot 2
And the publisher already wrote message 98 to slot 2!

Message 90 is gone, overwritten.
```

### The solution

Every message has a sequence number stamped in its header:

```
When publisher writes message 90 to slot 2:
  slot 2 header: sequence = 90

Later, publisher writes message 98 to slot 2:
  slot 2 header: sequence = 98  (overwrites)
```

Subscriber checks:

```
I want message 90.
I read slot 2.
Header says sequence = 98.
98 ≠ 90 → Data was overwritten!
```

When this happens, subscriber skips ahead to the oldest available message.

---

## Part 5: Futex (Efficient Waiting)

### The problem

Subscriber has read all available messages. Now what?

**Bad: Spin in a loop**
```cpp
while (read_idx >= write_idx) {
    // Burn CPU doing nothing
}
```

**Bad: Sleep and poll**
```cpp
while (read_idx >= write_idx) {
    sleep(10ms);  // Wastes 10ms before noticing new data
}
```

### The solution: Futex

Futex is a Linux feature. Think of it as a doorbell.

**Subscriber (waiting):**
```
"I'll go to sleep. Wake me when the doorbell rings."
→ Thread is truly asleep, using zero CPU
```

**Publisher (after writing):**
```
"New message available!"
→ Rings the doorbell
→ Kernel instantly wakes the sleeping subscriber
```

### How it works

There's a counter in shared memory called `futex_word`:

**Subscriber:**
1. Reads `futex_word` (say it's 50)
2. Checks for data — none available
3. Tells kernel: "Put me to sleep, but ONLY if futex_word is still 50"
4. Kernel checks: yep, still 50 → subscriber sleeps

**Publisher:**
1. Writes message
2. Increments `futex_word` (50 → 51)
3. Tells kernel: "Wake anyone sleeping on this address"
4. Kernel wakes subscriber

**Why the check matters:**

What if publisher writes between steps 1 and 3?

```
Subscriber: reads futex_word = 50
Publisher: writes message, futex_word = 51, tries to wake (nobody sleeping)
Subscriber: tells kernel "sleep if futex_word == 50"
Kernel: "It's 51 now, not 50. I won't put you to sleep."
Subscriber: wakes up immediately, sees the new message
```

No missed wakeups. No race conditions.

---

## Part 6: Memory Layout

Here's what the shared memory actually looks like:

```
┌─────────────────────────────────────────────────────────────────┐
│                        HEADER (~1 KB)                           │
│                                                                 │
│  Configuration (set once by publisher):                         │
│    slot_count = 16                                              │
│    slot_size = 4096                                             │
│                                                                 │
│  Coordination (updated during operation):                       │
│    write_idx = 1000        ← Publisher's counter                │
│    futex_word = 1000       ← Doorbell counter                   │
│    subscriber_mask = 0b111 ← 3 subscribers active               │
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

---

## Part 7: Complete Flow

Let's trace a message from publisher to subscriber.

### Publisher sends IMU data

```cpp
ImuData imu = {.accel = {0.1, 0.2, 9.8}, .gyro = {0, 0, 0.01}};
pub.publish(&imu, sizeof(imu));  // 48 bytes
```

**Step 1:** Check size
```
48 bytes + 20 byte header = 68 bytes
68 < 4096 slot size ✓
```

**Step 2:** Find slot
```
write_idx = 1000
slot = 1000 % 16 = 8
```

**Step 3:** Write to slot 8
```
Slot 8:
  bytes 0-3:   size = 48
  bytes 4-11:  sequence = 1000
  bytes 12-19: timestamp = [current time]
  bytes 20-67: [48 bytes of IMU data]
```

**Step 4:** Update write_idx
```
write_idx = 1001  (atomic write)
```

**Step 5:** Ring doorbell
```
futex_word++ → 1001
futex_wake_all()  → wake sleeping subscribers
```

### Subscriber receives

**Step 1:** Check for data
```
read_idx = 1000
write_idx = 1001
1001 > 1000 → new data available!
```

**Step 2:** Find slot
```
slot = 1000 % 16 = 8
```

**Step 3:** Verify not overwritten
```
slot 8 header: sequence = 1000
expected: 1000
1000 == 1000 ✓ Data is valid
```

**Step 4:** Return pointer
```
Return pointer to bytes 20-67 of slot 8
(This points directly into shared memory — zero copy!)
```

**Step 5:** Update read_idx
```
read_idx = 1001
```

---

## Summary

| Concept | What it is | Why it matters |
|---------|-----------|----------------|
| **Shared memory** | RAM both processes can access | Zero-copy — no data duplication |
| **Ring buffer** | Fixed slots that wrap around | Bounded memory, handles continuous data |
| **write_idx** | "Publisher wrote N messages" | Tells subscribers where new data is |
| **read_idx** | "Subscriber read up to N" | Each subscriber tracks own progress |
| **sequence** | Version stamp in each slot | Detects if slow subscriber missed data |
| **futex_word** | Doorbell counter | Efficient sleep/wake, zero CPU when idle |

**The elegance:** All coordination happens through a few integers in shared memory. No locks. No kernel calls except when sleeping. No copies of your data.

---

## Code Locations

| File | What it does |
|------|--------------|
| `src/internal/shm_region.cpp` | Create/open/delete shared memory |
| `src/internal/ring_buffer.cpp` | Read and write to slots |
| `src/internal/futex.cpp` | Sleep and wake |
| `src/internal/time.cpp` | Get timestamps |
| `src/pubsub.cpp` | Publisher and Subscriber classes |
| `src/node.cpp` | Node with threading and signals |
