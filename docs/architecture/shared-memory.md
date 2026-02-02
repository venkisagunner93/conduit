# Shared Memory

RAM that multiple processes can access. No copying between processes.

![Shared Memory](../images/shared_memory.svg)

## How it works

1. Publisher creates `/dev/shm/conduit_{topic}`
2. Subscriber opens the same file
3. Both see the same bytes in RAM

These aren't files on disk — they're named chunks of RAM.

```bash
$ ls /dev/shm/
conduit_imu      # Topic "imu"
conduit_camera   # Topic "camera"
```

---

**Next:** [Ring Buffer](ring-buffer.md) — How messages are organized in shared memory
