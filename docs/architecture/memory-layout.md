# Memory Layout

Each topic is a single shared memory file at `/dev/shm/conduit_{topic}`.

![Memory Layout](../images/memory_layout.svg)

## Structure

| Section | Contents | Size |
|---------|----------|------|
| **Header** | Config + write_idx + futex_word + read_idx[16] | ~1 KB |
| **Slots** | slot_count × slot_size | configurable |

## Cache-line alignment

Each `read_idx` gets its own 64-byte cache line. Without this, multiple CPUs updating different subscribers would fight over the same cache line ("false sharing").

## Total size

```
total = 1088 + (slot_count × slot_size)
```

| Config | Size |
|--------|------|
| 16 slots × 4 KB | ~65 KB |
| 4 slots × 1 MB (images) | ~4 MB |

---

