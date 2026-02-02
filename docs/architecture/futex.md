# Futex

How subscribers wait efficiently when there's no data.

![Futex](../images/futex.svg)

## The problem

Subscriber caught up, nothing to read. Three options:

| Approach | CPU | Latency |
|----------|-----|---------|
| Busy-wait | 100% | Instant |
| Sleep(10ms) | ~0% | Up to 10ms |
| **Futex** | **0%** | **~2-5 µs** |

## How futex works

**Subscriber:** "Sleep me, but only if `futex_word` is still 100."

**Publisher:** Writes message, increments `futex_word` to 101, calls `futex_wake()`.

**Kernel:** Sees 101 ≠ 100, wakes subscriber immediately.

No missed wakeups. No race conditions.

---

**Next:** [Memory Layout](memory-layout.md) — What the bytes actually look like
