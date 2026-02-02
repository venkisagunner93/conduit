# Ring Buffer

Fixed slots that wrap around. No memory growth, no corruption.

## How slots wrap

![Ring Buffer Wrapping](../images/ring_buffer_wrap.svg)

**Formula:** `slot = message_number % slot_count`

With 4 slots: message #0 → slot 0, message #4 → slot 0 (overwrites #0).

## Slot layout

![Slot Layout](../images/slot_layout.svg)

| Field | Size | Purpose |
|-------|------|---------|
| `size` | 4 bytes | Payload length |
| `sequence` | 8 bytes | Message number (for overwrite detection) |
| `timestamp` | 8 bytes | When published (nanoseconds) |
| `payload` | remaining | Your data |

**Subscriber gets:** pointer to byte 20 (your data), plus size/sequence/timestamp from header.

---

**Next:** [Indices](indices.md) — How publisher and subscriber coordinate
