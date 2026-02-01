# Write Index and Read Index

## All three numbers use the same scale

```
write_idx, read_idx, and sequence are ALL message numbers.

Message #0, Message #1, Message #2, ...
```

They all start at 0 and count up together:

| Message | sequence (in slot) | write_idx (after write) | read_idx (after read) |
|---------|-------------------|------------------------|----------------------|
| #0 | 0 | 0 | 0 |
| #1 | 1 | 1 | 1 |
| #2 | 2 | 2 | 2 |
| #3 | 3 | 3 | 3 |

## What each number means

```
write_idx = 3    →  "Publisher is about to write message #3"
                    (Messages #0, #1, #2 already exist)

read_idx = 1     →  "Subscriber wants to read message #1 next"
                    (Already read message #0)

sequence = 2     →  "This slot contains message #2"
                    (Stored inside the slot header)
```

## How they work together

```
New data available?

    read_idx < write_idx   →  YES
    read_idx == write_idx  →  NO, wait
```

## Step-by-step example

### Initial state

```
write_idx = 0     (no messages yet)
```

### Publisher writes message #0

```
1. write_idx is 0
2. Write to slot 0 with sequence = 0
3. Increment: write_idx = 1
```

```
AFTER:
  write_idx = 1
  Slot 0: sequence = 0, data = "Hello"
```

### Publisher writes message #1

```
1. write_idx is 1
2. Write to slot 1 with sequence = 1
3. Increment: write_idx = 2
```

```
AFTER:
  write_idx = 2
  Slot 0: sequence = 0, data = "Hello"
  Slot 1: sequence = 1, data = "World"
```

### Subscriber joins

```
Subscriber sees write_idx = 2
Subscriber sets read_idx = 2   (start fresh, don't read old messages)

Check: read_idx (2) < write_idx (2)?  NO → wait
```

### Publisher writes message #2

```
1. write_idx is 2
2. Write to slot 2 with sequence = 2
3. Increment: write_idx = 3
```

```
AFTER:
  write_idx = 3
  Slot 0: sequence = 0
  Slot 1: sequence = 1
  Slot 2: sequence = 2, data = "!"
```

### Subscriber reads

```
Check: read_idx (2) < write_idx (3)?  YES → new data!

1. Want message #2
2. Slot = 2 % 4 = slot 2
3. Read slot 2, verify sequence = 2  ✓
4. Increment: read_idx = 3

Check: read_idx (3) < write_idx (3)?  NO → wait
```

## The sequence number catches overwrites

With 4 slots, after 10 messages:

```
write_idx = 10

Slot 0: sequence = 8   (message #8, overwrote #0 and #4)
Slot 1: sequence = 9   (message #9, overwrote #1 and #5)
Slot 2: sequence = 6   (message #6, overwrote #2)
Slot 3: sequence = 7   (message #7, overwrote #3)
```

Slow subscriber with read_idx = 5:

```
1. Want message #5
2. Slot = 5 % 4 = slot 1
3. Read slot 1, sequence = 9
4. Expected 5, got 9 → OVERWRITTEN!
5. Skip ahead to oldest available (message #6)
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
/dev/shm/conduit_imu with 4 slots, after 10 messages:

┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 10      "About to write message #10"              │
│   read_idx[0] = 10    "Subscriber A: wants #10, waiting"        │
│   read_idx[1] = 8     "Subscriber B: wants #8"                  │
├──────────────────────────────┬──────────────────────────────────┤
│ Slot 0: sequence = 8         │ Slot 1: sequence = 9             │
├──────────────────────────────┼──────────────────────────────────┤
│ Slot 2: sequence = 6         │ Slot 3: sequence = 7             │
└──────────────────────────────┴──────────────────────────────────┘

Still available: messages 6, 7, 8, 9
Overwritten: messages 0, 1, 2, 3, 4, 5
```

**Subscriber A** (read_idx = 10):
```
10 < 10? NO → Caught up. Wait.
```

**Subscriber B** (read_idx = 8):
```
8 < 10? YES → Data available!
Slot = 8 % 4 = 0
Read slot 0: sequence = 8 ✓ (matches what we want)
Got message #8!
read_idx = 9
```

## Summary

| Value | What it means | Example |
|-------|---------------|---------|
| **write_idx** | Next message number to write | 10 = "about to write #10" |
| **read_idx** | Next message number to read | 8 = "want to read #8" |
| **sequence** | Which message is in this slot | 8 = "I am message #8" |

All three are message numbers on the same scale: 0, 1, 2, 3...

**Slot number** is different — it's `message_number % slot_count`

---

**Next:** [Sequence Numbers](sequence.md) — More on detecting overwrites
