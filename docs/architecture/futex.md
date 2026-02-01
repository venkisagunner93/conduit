# Futex (Efficient Waiting)

## The problem

Subscriber has read all available messages. Now what?

**Bad: Spin in a loop**
```cpp
while (read_idx >= write_idx) {
    // Burn CPU doing nothing
}
```
This uses 100% CPU waiting for data.

**Bad: Sleep and poll**
```cpp
while (read_idx >= write_idx) {
    sleep(10ms);  // Wastes 10ms before noticing new data
}
```
This adds up to 10ms latency.

## The solution: Futex

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

## How it works

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

## Why the check matters

What if publisher writes between steps 1 and 3?

```
Subscriber: reads futex_word = 50
Publisher: writes message, futex_word = 51, tries to wake (nobody sleeping)
Subscriber: tells kernel "sleep if futex_word == 50"
Kernel: "It's 51 now, not 50. I won't put you to sleep."
Subscriber: wakes up immediately, sees the new message
```

No missed wakeups. No race conditions.

## Visual example

```
SUBSCRIBER WAITING
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 100                                               │
│   futex_word = 100        ← Subscriber read this, now sleeping  │
│   read_idx[0] = 100       ← Caught up                           │
├─────────────────────────────────────────────────────────────────┤
│ ...slots...                                                     │
└─────────────────────────────────────────────────────────────────┘

Subscriber: "futex_word is 100. Sleep until it changes."
           → Thread sleeping in kernel, 0% CPU
```

```
PUBLISHER WRITES
═══════════════════════════════════════════════════════════════════

/dev/shm/conduit_imu:
┌─────────────────────────────────────────────────────────────────┐
│ HEADER                                                          │
│   write_idx = 101         ← Updated                             │
│   futex_word = 101        ← Incremented                         │
│   read_idx[0] = 100                                             │
├─────────────────────────────────────────────────────────────────┤
│ Slot 1: [seq=100][NEW DATA]                                     │
└─────────────────────────────────────────────────────────────────┘

Publisher: "futex_wake_all()"
           → Kernel wakes subscriber instantly

Subscriber: "I'm awake! write_idx (101) > read_idx (100). Read!"
```

## The key benefits

| Approach | CPU while waiting | Wake latency |
|----------|-------------------|--------------|
| Busy-wait | 100% | Instant |
| Sleep(10ms) | ~0% | Up to 10ms |
| **Futex** | **0%** | **~2-5 µs** |

Futex gives you the best of both worlds.

---

**Next:** [Memory Layout](memory-layout.md) — What the bytes actually look like
