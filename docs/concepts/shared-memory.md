# Shared Memory

Shared memory is the foundation of Conduit's zero-copy design.

## What is Shared Memory?

Normally, when two programs share data, they copy it:

```
Program A → [copy to kernel] → [copy to Program B]
```

Shared memory eliminates these copies. Both programs see the **same physical memory**:

```
Program A's address space:  [....][SHARED][....]
                                   |
                                   v  (same physical RAM)
Program B's address space:  [....][SHARED][....]
```

## How Linux Implements It

On Linux, shared memory lives in `/dev/shm/` - a RAM-based filesystem (tmpfs).

```bash
# After creating a topic named "imu":
$ ls -la /dev/shm/conduit_*
-rw-rw-rw- 1 user user 69632 Jan 15 10:30 /dev/shm/conduit_imu
```

The file exists in RAM, not on disk. It's fast and disappears on reboot.

## Creating Shared Memory

```cpp
// Publisher creates the region
ShmRegion region = ShmRegion::create("imu", 69632);
void* ptr = region.data();  // Pointer to shared memory
```

Behind the scenes:

1. **shm_open()** - Creates `/dev/shm/conduit_imu`
2. **ftruncate()** - Sets size (allocates RAM)
3. **mmap()** - Maps into address space (gives us a pointer)

## Opening Shared Memory

```cpp
// Subscriber opens existing region
ShmRegion region = ShmRegion::open("imu");
void* ptr = region.data();  // Points to SAME memory!
```

## Example: Sharing an Integer

**Publisher:**
```cpp
ShmRegion region = ShmRegion::create("counter", 4096);
int* ptr = static_cast<int*>(region.data());
*ptr = 42;  // Write to shared memory
```

**Subscriber:**
```cpp
ShmRegion region = ShmRegion::open("counter");
int* ptr = static_cast<int*>(region.data());
int value = *ptr;  // Reads 42!
```

No copying - both processes access the same RAM.

## Example: Sharing a Point Cloud

**Publisher:**
```cpp
struct PointCloud {
    float x[1000000];  // 4 MB
    float y[1000000];  // 4 MB
    float z[1000000];  // 4 MB
};  // Total: 12 MB

ShmRegion region = ShmRegion::create("lidar", 13'000'000);
PointCloud* cloud = static_cast<PointCloud*>(region.data());
// Fill cloud->x, cloud->y, cloud->z
```

**Subscriber:**
```cpp
ShmRegion region = ShmRegion::open("lidar");
PointCloud* cloud = static_cast<PointCloud*>(region.data());
float first_x = cloud->x[0];  // Direct access, no copy!
```

A 12MB point cloud is never copied between processes.

## API Reference

| Function | Description |
|----------|-------------|
| `ShmRegion::create(name, size)` | Create new shared memory |
| `ShmRegion::open(name)` | Open existing shared memory |
| `ShmRegion::exists(name)` | Check if topic exists |
| `ShmRegion::wait_until_exists(name, flag)` | Wait for topic to appear |
| `ShmRegion::unlink(name)` | Delete shared memory file |
| `region.data()` | Get pointer to memory |
| `region.size()` | Get size in bytes |

## Important Notes

!!! warning "Single Publisher"
    Each topic can only have one publisher. `ShmRegion::create()` fails if
    the topic already exists. This enforces the single-producer design.

!!! info "Persistence"
    Shared memory persists until:

    - Explicitly unlinked with `ShmRegion::unlink()`
    - System reboot

    Crashed processes don't clean up their topics.

!!! tip "Memory Alignment"
    For best performance with large data structures, ensure proper alignment.
    The ring buffer header is cache-line aligned (64 bytes).
