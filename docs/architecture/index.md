# Architecture

This section explains how Conduit works, piece by piece.

## The Core Idea

Imagine a whiteboard in a shared office. Anyone in the office can read it. One person writes on it.

That's Conduit.

- **The whiteboard** = shared memory (a chunk of RAM both processes can access)
- **The writer** = publisher
- **The readers** = subscribers

No photocopying. No passing notes. Everyone looks at the same whiteboard.

## The Building Blocks

| Part | What it explains |
|------|------------------|
| [Shared Memory](shared-memory.md) | How two processes see the same RAM |
| [Ring Buffer](ring-buffer.md) | How messages are stored in slots |
| [Indices](indices.md) | How publisher and subscriber coordinate |
| [Sequence Numbers](sequence.md) | How we detect overwritten data |
| [Futex](futex.md) | How subscribers sleep efficiently |
| [Memory Layout](memory-layout.md) | What the bytes actually look like |

**The elegance:** All coordination happens through a few integers in shared memory. No locks. No kernel calls except when sleeping. No copies of your data.

## Code Locations

| File | What it does |
|------|--------------|
| `src/internal/shm_region.cpp` | Create/open/delete shared memory |
| `src/internal/ring_buffer.cpp` | Read and write to slots |
| `src/internal/futex.cpp` | Sleep and wake |
| `src/internal/time.cpp` | Get timestamps |
| `src/pubsub.cpp` | Publisher and Subscriber classes |
| `src/node.cpp` | Node with threading and signals |
