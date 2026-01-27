# Publisher

Publish messages to a topic.

## Basic Usage

```cpp
#include <conduit_core/pubsub.hpp>

conduit::Publisher pub("my_topic");
pub.publish(data_ptr, data_size);
```

## With Node

When using the Node class, create publishers via `advertise()`:

```cpp
#include <conduit_core/node.hpp>
#include <optional>

class MyNode : public conduit::Node {
public:
    MyNode() {
        // Publisher has no default constructor, use std::optional
        pub_.emplace(advertise("output"));
    }

private:
    void send_data() {
        MyStruct data = {...};
        pub_->publish(&data, sizeof(data));
    }

    std::optional<conduit::Publisher> pub_;
};
```

## API

### Constructor

```cpp
Publisher(const std::string& topic);
Publisher(const std::string& topic, const RingBufferConfig& config);
```

Creates shared memory at `/dev/shm/conduit_{topic}`.

**Only one publisher per topic.** Creating a second publisher for the same topic throws an exception.

### publish()

```cpp
bool publish(const void* data, size_t size);
```

Writes data to the ring buffer and wakes subscribers.

| Parameter | Description |
|-----------|-------------|
| `data` | Pointer to message bytes |
| `size` | Number of bytes |

**Returns:** `true` if published, `false` if message too large for slot.

### topic()

```cpp
const std::string& topic() const;
```

Returns the topic name.

## Configuration

```cpp
RingBufferConfig config;
config.slot_count = 16;      // Number of message slots (power of 2)
config.slot_size = 4096;     // Bytes per slot (header + payload)

Publisher pub("camera", config);
```

| Setting | Default | Description |
|---------|---------|-------------|
| `slot_count` | 16 | How many messages to buffer |
| `slot_size` | 4096 | Max message size + 20 bytes |

**Choosing slot_size:**

Your largest message must fit: `slot_size >= max_message_size + 20`

| Data | Typical Size | Suggested slot_size |
|------|--------------|---------------------|
| IMU | 48 bytes | 4 KB (default) |
| Pose | 56 bytes | 4 KB (default) |
| Compressed image | 100 KB | 128 KB |
| Point cloud | 1-10 MB | 12 MB |
| Raw camera | 2 MB | 4 MB |

## Examples

### Simple String Publisher

```cpp
#include <conduit_core/node.hpp>
#include <conduit_core/log.hpp>

class StringPublisher : public conduit::Node {
public:
    StringPublisher() {
        pub_.emplace(advertise("messages"));
        loop(1.0, &StringPublisher::publish);
    }

private:
    void publish() {
        std::string msg = "Hello #" + std::to_string(count_++);
        pub_->publish(msg.data(), msg.size());
        conduit::log::info("Sent: {}", msg);
    }

    std::optional<conduit::Publisher> pub_;
    uint64_t count_ = 0;
};
```

### Struct Publisher

```cpp
struct ImuData {
    double timestamp;
    float accel[3];
    float gyro[3];
};

class ImuPublisher : public conduit::Node {
public:
    ImuPublisher() {
        pub_.emplace(advertise("imu"));
        loop(100.0, &ImuPublisher::publish);  // 100 Hz
    }

private:
    void publish() {
        ImuData data;
        data.timestamp = get_time();
        read_sensor(data.accel, data.gyro);
        pub_->publish(&data, sizeof(data));
    }

    std::optional<conduit::Publisher> pub_;
};
```

### Large Message Publisher

```cpp
class CameraPublisher : public conduit::Node {
public:
    CameraPublisher() {
        // Configure for 2 MB images
        RingBufferConfig config;
        config.slot_count = 4;
        config.slot_size = 2'100'000;

        pub_.emplace(advertise("camera", config));
        loop(30.0, &CameraPublisher::publish);  // 30 FPS
    }

private:
    void publish() {
        std::vector<uint8_t> image = capture_frame();
        pub_->publish(image.data(), image.size());
    }

    std::optional<conduit::Publisher> pub_;
};
```

## What Happens When You Publish

1. Check message fits in slot
2. Calculate slot index: `write_idx % slot_count`
3. Write 20-byte header (size, sequence, timestamp)
4. `memcpy` payload into slot
5. Increment `write_idx` (atomic)
6. Wake all sleeping subscribers (futex)

Total time: ~200-500 ns for small messages, dominated by `memcpy` for large ones.
