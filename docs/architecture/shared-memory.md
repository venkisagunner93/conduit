# Shared Memory

## What is it?

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

## How Conduit uses it

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

**Next:** [Ring Buffer](ring-buffer.md) — How messages are organized in shared memory
