# Publisher

Publish typed messages to a topic.

## Basic Usage

```cpp
#include <conduit_core/publisher.hpp>
#include <conduit_types/primitives/vec3.hpp>

conduit::Publisher<conduit::Vec3> pub("imu_accel");

conduit::Vec3 data{};
data.x = 0.1;
data.y = 0.0;
data.z = 9.81;
pub.publish(data);
```

## With Node

When using the Node class, create publishers via `advertise<T>()`:

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/primitives/vec3.hpp>

using namespace conduit;

class MyNode : public Node {
public:
    MyNode() {
        pub_.emplace(advertise<Vec3>("output"));
    }

private:
    void send_data() {
        Vec3 data{};
        data.x = 1.0;
        pub_->publish(data);
    }

    std::optional<Publisher<Vec3>> pub_;
};
```

## API

### Constructor

```cpp
Publisher<T>(const std::string& topic, const PublisherOptions& options = {});
```

Creates shared memory at `/dev/shm/conduit_{topic}`.

**Only one publisher per topic.** Creating a second publisher for the same topic throws an exception.

The type `T` must be either a `FixedMessageType` or a `VariableMessageType`. This is validated at compile time.

### publish()

```cpp
bool publish(const T& msg);
```

Publishes a typed message to the topic.

- **Fixed types** (`FixedMessageType`): Sent via `memcpy` — zero serialization overhead.
- **Variable types** (`VariableMessageType`): Serialized via the type's `serialize()` method.

**Returns:** `true` if published, `false` if message too large for slot.

### topic()

```cpp
const std::string& topic() const;
```

Returns the topic name.

### max_message_size()

```cpp
uint32_t max_message_size() const;
```

Returns the maximum payload size in bytes.

## Configuration

```cpp
conduit::PublisherOptions options;
options.depth = 16;              // Number of message slots (power of 2)
options.max_message_size = 4096; // Max payload size in bytes

Publisher<Vec3> pub("camera", options);
```

| Setting | Default | Description |
|---------|---------|-------------|
| `depth` | 16 | How many messages to buffer |
| `max_message_size` | 4096 | Max payload size in bytes |

**Choosing max_message_size:**

Your largest message must fit in this size.

| Data | Typical Size | Suggested max_message_size |
|------|--------------|----------------------------|
| IMU | 48 bytes | 4 KB (default) |
| Pose | 56 bytes | 4 KB (default) |
| Compressed image | 100 KB | 128 KB |
| Point cloud | 1-10 MB | 12 MB |
| Raw camera | 2 MB | 4 MB |

## Message Types

### Fixed Messages

Fixed-size types sent via `memcpy`. Derive from `FixedMessageType`:

```cpp
#include <conduit_types/fixed_message_type.hpp>

struct MyData : conduit::FixedMessageType {
    double x;
    double y;
    double z;
    uint64_t timestamp;
};
```

Requirements:
- Must be trivially copyable (no pointers, no `std::string`, no virtuals)
- Must be standard layout

See [Types](types.md) for built-in types.

### Variable Messages

Variable-size types that need serialization. Derive from `VariableMessageType`:

```cpp
#include <conduit_types/variable_message_type.hpp>
#include <conduit_types/buffer.hpp>

struct MyStringMsg : conduit::VariableMessageType {
    std::string text;

    size_t serialized_size() const override {
        return conduit::WriteBuffer::size_of(text);
    }

    void serialize(uint8_t* buffer) const override {
        conduit::WriteBuffer(buffer).write(text);
    }

    static MyStringMsg deserialize(const uint8_t* data, size_t size) {
        MyStringMsg msg;
        msg.text = conduit::ReadBuffer(data, size).read<std::string>();
        return msg;
    }
};
```

## Examples

### Struct Publisher

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/imu.hpp>

using namespace conduit;

class ImuPublisher : public Node {
public:
    ImuPublisher() {
        pub_.emplace(advertise<Imu>("imu"));
        loop(100.0, &ImuPublisher::publish);  // 100 Hz
    }

private:
    void publish() {
        Imu data{};
        set_frame(data.header.frame, "imu_link");
        data.linear_acceleration.z = 9.81;
        pub_->publish(data);
    }

    std::optional<Publisher<Imu>> pub_;
};
```

### Large Message Publisher

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/fixed_message_type.hpp>

using namespace conduit;

struct ImageData : FixedMessageType {
    uint8_t pixels[1920 * 1080 * 3];
};

class CameraPublisher : public Node {
public:
    CameraPublisher() {
        PublisherOptions options;
        options.depth = 4;
        options.max_message_size = sizeof(ImageData);

        pub_.emplace(advertise<ImageData>("camera", options));
        loop(30.0, &CameraPublisher::publish);  // 30 FPS
    }

private:
    void publish() {
        ImageData frame{};
        capture(frame.pixels);
        pub_->publish(frame);
    }

    std::optional<Publisher<ImageData>> pub_;
};
```

### Odometry Publisher

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/odometry.hpp>

using namespace conduit;

class OdomPublisher : public Node {
public:
    OdomPublisher() {
        pub_.emplace(advertise<Odometry>("odom"));
        loop(50.0, &OdomPublisher::publish);
    }

private:
    void publish() {
        Odometry odom{};
        set_frame(odom.header.frame, "odom");
        set_frame(odom.child_frame, "base_link");
        odom.pose.position.x = x_;
        odom.pose.orientation = Orientation::from_yaw(theta_);
        odom.linear_velocity.x = v_;
        pub_->publish(odom);
    }

    std::optional<Publisher<Odometry>> pub_;
    double x_ = 0, theta_ = 0, v_ = 0;
};
```

## What Happens When You Publish

1. Check message fits in slot
2. Calculate slot index: `write_idx % slot_count`
3. Write 20-byte header (size, sequence, timestamp)
4. `memcpy` payload into slot (fixed types) or write serialized bytes (variable types)
5. Increment `write_idx` (atomic)
6. Wake all sleeping subscribers (futex)

Total time: ~200-500 ns for small messages, dominated by `memcpy` for large ones.
