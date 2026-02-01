# Sequence Numbers (Overwrite Detection)

## The problem

What if a subscriber is too slow?

```
4 slots, subscriber is way behind:

write_idx = 100
read_idx = 90   ← Subscriber wants message 90

But slot = 90 % 4 = slot 2
And the publisher already wrote message 98 to slot 2!

Message 90 is gone, overwritten.
```

If the subscriber just reads slot 2, it would get message 98's data but think it's message 90. That's bad.

## The solution

Every message has a sequence number stamped in its header:

```
When publisher writes message 90 to slot 2:
  slot 2 header: sequence = 90

Later, publisher writes message 98 to slot 2:
  slot 2 header: sequence = 98  (overwrites)
```

## How subscriber detects overwrites

```
I want message 90.
I read slot 2.
Header says sequence = 98.
98 ≠ 90 → Data was overwritten!
```

When this happens, subscriber skips ahead to the oldest available message.

## Visual example

```
SUBSCRIBER IS BEHIND
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 100                                               │
│   read_idx[0] = 90        ← Subscriber wants msg 90             │
├─────────────────────────────────────────────────────────────────┤
│ Slot 0: [seq=96][...]                                           │
│ Slot 1: [seq=97][...]                                           │
│ Slot 2: [seq=98][...]     ← Subscriber checks: 98 ≠ 90 → STALE! │
│ Slot 3: [seq=99][...]                                           │
└─────────────────────────────────────────────────────────────────┘

Subscriber: "I missed messages 90-95. Skip to oldest available."
Subscriber sets read_idx = 96
```

## What "oldest available" means

With 4 slots and write_idx = 100:

- Newest message: 99 (in slot 99 % 4 = slot 3)
- Oldest message still available: 96 (in slot 96 % 4 = slot 0)
- Messages 0-95: gone, overwritten

```
Oldest available = write_idx - slot_count
                 = 100 - 4
                 = 96
```

---

**Next:** [Futex](futex.md) — How subscribers sleep efficiently
