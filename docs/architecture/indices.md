# Write Index and Read Index

## The simple version

Think of it like a ticket counter at a deli:

- **write_idx** = "Now serving ticket #X" (updated by publisher)
- **read_idx** = "I have ticket #Y" (each subscriber has their own)

If your ticket number equals the "now serving" number, you're caught up. Wait for the next ticket.

If your ticket number is less than "now serving", you missed some — go get served!

## What the numbers mean

```
write_idx = 5
```
This means: "The publisher has written 5 messages total (messages 0, 1, 2, 3, 4)."

The NEXT message will be message #5.

```
read_idx = 3
```
This means: "This subscriber has read up to message 2. It wants message #3 next."

## How they work together

```
Is there new data?

    read_idx < write_idx  →  YES, new data available
    read_idx == write_idx →  NO, caught up, wait
```

**Example:**
```
write_idx = 5 (messages 0-4 exist)
read_idx = 3 (subscriber has read 0, 1, 2)

3 < 5 → New data! Subscriber reads message 3.
After reading: read_idx = 4

4 < 5 → More data! Subscriber reads message 4.
After reading: read_idx = 5

5 < 5? NO (5 == 5) → Caught up. Wait.
```

## Step-by-step example

Let's trace from the beginning:

### 1. Publisher starts (no messages yet)

```
write_idx = 0    "Zero messages written"
```

### 2. Publisher sends message

```
Publisher writes message #0 to slot 0
write_idx = 1    "One message written (message 0)"
```

### 3. Publisher sends another

```
Publisher writes message #1 to slot 1
write_idx = 2    "Two messages written (messages 0, 1)"
```

### 4. Subscriber joins

```
Subscriber sees write_idx = 2
Subscriber sets read_idx = 2    "I'll start from the next one"

Check: read_idx (2) < write_idx (2)? NO
Subscriber waits...
```

### 5. Publisher sends another

```
Publisher writes message #2 to slot 2
write_idx = 3    "Three messages written"

Subscriber wakes up!
Check: read_idx (2) < write_idx (3)? YES!
Subscriber reads message #2
read_idx = 3

Check: read_idx (3) < write_idx (3)? NO
Subscriber waits...
```

## Where does the sequence number fit in?

The **sequence number** is stored INSIDE each message slot. It's a copy of write_idx at the time of writing.

```
When publisher writes message #2:
  - write_idx is currently 2
  - Publisher writes to slot (2 % 4 = slot 2)
  - Slot 2 header: sequence = 2
  - Then: write_idx = 3
```

**Why do we need it?**

The sequence number detects if a slot was overwritten. Here's when it matters:

```
4 slots, publisher has written 10 messages:

write_idx = 10
Slow subscriber: read_idx = 6

Subscriber wants message #6.
Slot = 6 % 4 = slot 2

But wait — the publisher already wrote message #10 to slot 2!
(10 % 4 = 2)

Slot 2 now contains message #10, not message #6.
```

**How subscriber detects this:**

```
Subscriber reads slot 2.
Slot 2 header says: sequence = 10
Subscriber expected: sequence = 6

10 ≠ 6 → "This slot was overwritten! I missed messages 6-9."
```

## Where does futex_word fit in?

The **futex_word** is completely separate. It's just for sleep/wake.

```
futex_word = "How many times has the publisher said 'new data!'?"
```

Every time publisher writes a message:
1. Write the message
2. Increment futex_word
3. Wake sleeping subscribers

The subscriber uses it to sleep efficiently:

```
Subscriber: "futex_word is 50. Wake me when it changes."
            *sleeps*

Publisher: "New message! futex_word = 51. Wake everyone!"

Subscriber: *wakes up* "Let me check write_idx for actual data."
```

**futex_word is NOT the message count.** It's just a "something changed" signal.

## Putting it all together

```
/dev/shm/conduit_imu with 4 slots:

write_idx = 10 means messages 0, 1, 2, ..., 9 have been written.

Message 6 → slot 6 % 4 = slot 2 → sequence = 6
Message 7 → slot 7 % 4 = slot 3 → sequence = 7
Message 8 → slot 8 % 4 = slot 0 → sequence = 8  (overwrote msg 4)
Message 9 → slot 9 % 4 = slot 1 → sequence = 9  (overwrote msg 5)

┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 10      "Next message will be #10"                │
│   futex_word = 10                                               │
│   read_idx[0] = 10    "Subscriber A: waiting for #10"           │
│   read_idx[1] = 8     "Subscriber B: wants #8"                  │
├──────────────────────────────┬──────────────────────────────────┤
│ Slot 0: sequence = 8         │ Slot 1: sequence = 9             │
│ (message #8)                 │ (message #9)                     │
├──────────────────────────────┼──────────────────────────────────┤
│ Slot 2: sequence = 6         │ Slot 3: sequence = 7             │
│ (message #6)                 │ (message #7)                     │
└──────────────────────────────┴──────────────────────────────────┘

Available messages: 6, 7, 8, 9 (oldest 4 messages)
Gone forever: 0, 1, 2, 3, 4, 5 (overwritten)
```

**Subscriber A** (read_idx = 10):
```
10 < 10? NO → Caught up. Sleep on futex.
```

**Subscriber B** (read_idx = 8):
```
8 < 10? YES → Data available!
Read message #8 from slot (8 % 4 = 0)
Check: slot 0 sequence (8) == expected (8)? YES ✓
Got message #8!
read_idx = 9
```

## Summary

| Value | Meaning | Who updates it |
|-------|---------|----------------|
| **write_idx** | Next message number to be written | Publisher (after each write) |
| **read_idx** | Next message number subscriber wants | Each subscriber (after each read) |
| **sequence** | "I am message #N" (stored in slot) | Publisher (during write) |
| **futex_word** | "Wake up!" signal counter | Publisher (after write) |

**The key insight:**

- `write_idx` and `read_idx` are message NUMBERS, not slot numbers
- Slot number = message_number % slot_count
- `sequence` inside a slot tells you which message is actually there
- `futex_word` is just for efficient sleeping, nothing to do with message numbers

---

**Next:** [Sequence Numbers](sequence.md) — More on detecting overwrites
