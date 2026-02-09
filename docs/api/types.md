# Types

Conduit provides a type system for messages via the `conduit_types` package. There are two categories: **fixed** (zero-copy via memcpy) and **variable** (serialized).

## Header

Most derived types include a `Header` for timestamping and coordinate frame identification.

```cpp
struct Header {
    uint64_t timestamp_ns;  // Timestamp in nanoseconds
    char frame[64];         // Coordinate frame identifier (null-terminated)
};
```

Use `set_frame()` to safely set the frame string:

```cpp
conduit::Imu imu{};
conduit::set_frame(imu.header.frame, "imu_link");
```

## Primitives

Simple fixed-size value types. All derive from `FixedMessageType`.

### Bool

```cpp
#include <conduit_types/primitives/bool.hpp>

struct Bool : FixedMessageType {
    bool value;
};
```

### Int

```cpp
#include <conduit_types/primitives/int.hpp>

struct Int : FixedMessageType {
    int64_t value;
};
```

### Uint

```cpp
#include <conduit_types/primitives/uint.hpp>

struct Uint : FixedMessageType {
    uint64_t value;
};
```

### Double

```cpp
#include <conduit_types/primitives/double.hpp>

struct Double : FixedMessageType {
    double value;
};
```

### Time

```cpp
#include <conduit_types/primitives/time.hpp>

struct Time : FixedMessageType {
    uint64_t nanoseconds;
};
```

### Vec2

```cpp
#include <conduit_types/primitives/vec2.hpp>

struct Vec2 : FixedMessageType {
    double x;
    double y;
};
```

### Vec3

```cpp
#include <conduit_types/primitives/vec3.hpp>

struct Vec3 : FixedMessageType {
    double x;
    double y;
    double z;
};
```

## Derived Types

Composite types built from primitives. All are fixed-size and include a `Header`.

### Orientation

Quaternion orientation with Euler angle conversions.

```cpp
#include <conduit_types/derived/orientation.hpp>

struct Orientation : FixedMessageType {
    double x;  // Quaternion X
    double y;  // Quaternion Y
    double z;  // Quaternion Z
    double w;  // Quaternion W (scalar)
};
```

**Conversion methods:**

```cpp
// From Euler angles (roll, pitch, yaw in radians)
auto q = Orientation::from_euler(roll, pitch, yaw);
auto q = Orientation::from_euler(roll, pitch, yaw, EulerOrder::XYZ);

// From yaw only (2D convenience)
auto q = Orientation::from_yaw(1.57);

// Back to Euler angles -> Vec3{roll, pitch, yaw}
Vec3 euler = q.to_euler();
Vec3 euler = q.to_euler(EulerOrder::XYZ);

// Extract yaw (2D convenience)
double yaw = q.to_yaw();
```

**Euler orders:** `EulerOrder::ZYX` (default, aerospace) and `EulerOrder::XYZ`.

### Pose2D

```cpp
#include <conduit_types/derived/pose2d.hpp>

struct Pose2D : FixedMessageType {
    Header header;
    Vec2 position;
    Orientation orientation;
};
```

### Pose3D

```cpp
#include <conduit_types/derived/pose3d.hpp>

struct Pose3D : FixedMessageType {
    Header header;
    Vec3 position;
    Orientation orientation;
};
```

### Twist

Linear and angular velocity.

```cpp
#include <conduit_types/derived/twist.hpp>

struct Twist : FixedMessageType {
    Header header;
    Vec3 linear;   // Linear velocity (m/s)
    Vec3 angular;  // Angular velocity (rad/s)
};
```

### Imu

Inertial measurement unit data.

```cpp
#include <conduit_types/derived/imu.hpp>

struct Imu : FixedMessageType {
    Header header;
    Orientation orientation;
    Vec3 angular_velocity;     // rad/s
    Vec3 linear_acceleration;  // m/s^2
};
```

### Odometry

Full odometry with pose and velocities.

```cpp
#include <conduit_types/derived/odometry.hpp>

struct Odometry : FixedMessageType {
    Header header;
    char child_frame[64];
    Pose3D pose;
    Vec3 linear_velocity;   // m/s
    Vec3 angular_velocity;  // rad/s
};
```

**Example usage:**

```cpp
Odometry odom{};
set_frame(odom.header.frame, "odom");
set_frame(odom.child_frame, "base_link");
odom.pose.position.x = 1.0;
odom.pose.orientation = Orientation::from_yaw(0.5);
odom.linear_velocity.x = 1.0;
```

## Creating Custom Types

### Fixed Message Type

Fixed types are transported via `memcpy` with zero serialization overhead. They must be trivially copyable, standard layout, and non-empty.

**Rules:**

1. Derive from `conduit::FixedMessageType`
2. Only use plain data fields (no pointers, `std::string`, `std::vector`, or virtual functions)
3. You can compose built-in conduit types freely

```cpp
#include <conduit_types/fixed_message_type.hpp>
#include <conduit_types/header.hpp>
#include <conduit_types/primitives/vec3.hpp>

struct MotorCommand : conduit::FixedMessageType {
    uint32_t motor_id;
    double velocity;
    double torque;
};

struct RobotState : conduit::FixedMessageType {
    conduit::Header header;
    conduit::Vec3 position;
    conduit::Vec3 velocity;
    double battery_voltage;
    uint8_t mode;
};
```

**Publish and subscribe:**

```cpp
// Publisher
conduit::Publisher<MotorCommand> pub("motor_cmd");
MotorCommand cmd{};
cmd.motor_id = 1;
cmd.velocity = 1.5;
cmd.torque = 0.3;
pub.publish(cmd);

// Subscriber
conduit::Subscriber<MotorCommand> sub("motor_cmd");
auto msg = sub.wait();
// msg.data.motor_id, msg.data.velocity, msg.data.torque
// msg.sequence, msg.timestamp_ns
```

### Variable Message Type

Variable types contain dynamic-size data (strings, vectors, maps) and require explicit serialization. Use `WriteBuffer` and `ReadBuffer` to handle the byte packing.

**Rules:**

1. Derive from `conduit::VariableMessageType`
2. Implement `serialized_size()` — returns total byte count
3. Implement `serialize(uint8_t* buffer)` — writes fields into the buffer
4. Provide `static T deserialize(const uint8_t* data, size_t size)` — reconstructs from bytes

```cpp
#include <conduit_types/variable_message_type.hpp>
#include <conduit_types/buffer.hpp>

struct LogEntry : conduit::VariableMessageType {
    uint32_t level;
    std::string message;

    size_t serialized_size() const override {
        return conduit::WriteBuffer::size_of(level)
             + conduit::WriteBuffer::size_of(message);
    }

    void serialize(uint8_t* buffer) const override {
        conduit::WriteBuffer buf(buffer);
        buf.write(level);
        buf.write(message);
    }

    static LogEntry deserialize(const uint8_t* data, size_t size) {
        conduit::ReadBuffer buf(data, size);
        LogEntry entry;
        entry.level = buf.read<uint32_t>();
        entry.message = buf.read<std::string>();
        return entry;
    }
};
```

**Publish and subscribe:**

```cpp
// Publisher
conduit::Publisher<LogEntry> pub("logs");
LogEntry entry;
entry.level = 2;
entry.message = "Motor overheating on joint 3";
pub.publish(entry);

// Subscriber
conduit::Subscriber<LogEntry> sub("logs");
auto msg = sub.wait();
// msg.data.level, msg.data.message
```

## Serialization Helpers

`WriteBuffer` and `ReadBuffer` handle the byte-level packing for variable message types. Strings are stored with a 4-byte length prefix. Trivially copyable values are stored directly via memcpy.

### WriteBuffer

```cpp
#include <conduit_types/buffer.hpp>

// Calculate total serialized size
size_t size = WriteBuffer::size_of(my_string)   // 4 + string length
            + WriteBuffer::size_of(my_double);   // sizeof(double)

// Write into a buffer
WriteBuffer buf(buffer_ptr);
buf.write(my_string);   // length-prefixed string
buf.write(my_double);   // raw bytes via memcpy
```

### ReadBuffer

```cpp
ReadBuffer buf(data_ptr, data_size);
auto s = buf.read<std::string>();   // reads length-prefixed string
auto d = buf.read<double>();        // reads sizeof(double) bytes
```

Fields must be read in the same order they were written.
