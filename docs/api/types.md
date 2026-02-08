# Types

Conduit provides a type system for messages via the `conduit_types` package.

## Message Categories

### Fixed Messages

Fixed-size types transported via `memcpy` with zero serialization overhead. Derive from `FixedMessageType`:

```cpp
#include <conduit_types/fixed_message_type.hpp>

struct MyData : conduit::FixedMessageType {
    double x;
    double y;
    uint32_t flags;
};
```

**Requirements:**

- Trivially copyable (no pointers, `std::string`, virtual functions)
- Standard layout
- Non-empty

All built-in primitives and derived types are fixed message types.

### Variable Messages

Variable-size types that require serialization. Derive from `VariableMessageType`:

```cpp
#include <conduit_types/variable_message_type.hpp>

struct MyMsg : conduit::VariableMessageType {
    std::string name;
    std::vector<double> values;

    size_t serialized_size() const override { /* ... */ }
    void serialize(uint8_t* buffer) const override { /* ... */ }
    static MyMsg deserialize(const uint8_t* data, size_t size) { /* ... */ }
};
```

**Requirements:**

- Implement `serialized_size()` and `serialize()`
- Provide static `deserialize(const uint8_t*, size_t) -> T`

Use `WriteBuffer` and `ReadBuffer` helpers for serialization (see below).

## Primitives

Simple value types in `conduit_types/primitives/`.

| Type | Header | Fields |
|------|--------|--------|
| `Bool` | `primitives/bool.hpp` | `bool value` |
| `Int` | `primitives/int.hpp` | `int64_t value` |
| `Uint` | `primitives/uint.hpp` | `uint64_t value` |
| `Double` | `primitives/double.hpp` | `double value` |
| `Time` | `primitives/time.hpp` | `uint64_t nanoseconds` |
| `Vec2` | `primitives/vec2.hpp` | `double x, y` |
| `Vec3` | `primitives/vec3.hpp` | `double x, y, z` |

```cpp
#include <conduit_types/primitives/vec3.hpp>

conduit::Vec3 v{};
v.x = 1.0;
v.y = 2.0;
v.z = 3.0;
```

## Header

Most derived types include a `Header` for timestamping and frame identification:

```cpp
#include <conduit_types/header.hpp>

struct Header {
    uint64_t timestamp_ns;
    char frame[64];
};
```

Use `set_frame()` to safely set the frame string:

```cpp
conduit::Imu imu{};
conduit::set_frame(imu.header.frame, "imu_link");
```

## Derived Types

Composite types built from primitives. All include a `Header`. Located in `conduit_types/derived/`.

### Orientation

Quaternion orientation with Euler angle conversions.

```cpp
#include <conduit_types/derived/orientation.hpp>
```

| Field | Type |
|-------|------|
| `x` | `double` |
| `y` | `double` |
| `z` | `double` |
| `w` | `double` |

**Conversion methods:**

```cpp
// Create from Euler angles (roll, pitch, yaw)
auto q = Orientation::from_euler(roll, pitch, yaw);
auto q = Orientation::from_euler(roll, pitch, yaw, EulerOrder::XYZ);

// Create from yaw only (2D convenience)
auto q = Orientation::from_yaw(1.57);

// Convert back to Euler angles -> Vec3{roll, pitch, yaw}
Vec3 euler = q.to_euler();
Vec3 euler = q.to_euler(EulerOrder::XYZ);

// Extract yaw (2D convenience)
double yaw = q.to_yaw();
```

**Euler orders:** `EulerOrder::ZYX` (default, aerospace convention) and `EulerOrder::XYZ`.

### Pose2D

2D position and orientation.

```cpp
#include <conduit_types/derived/pose2d.hpp>
```

| Field | Type |
|-------|------|
| `header` | `Header` |
| `position` | `Vec2` |
| `orientation` | `Orientation` |

### Pose3D

3D position and orientation.

```cpp
#include <conduit_types/derived/pose3d.hpp>
```

| Field | Type |
|-------|------|
| `header` | `Header` |
| `position` | `Vec3` |
| `orientation` | `Orientation` |

### Twist

Linear and angular velocity.

```cpp
#include <conduit_types/derived/twist.hpp>
```

| Field | Type |
|-------|------|
| `header` | `Header` |
| `linear` | `Vec3` |
| `angular` | `Vec3` |

### Imu

Inertial measurement unit data.

```cpp
#include <conduit_types/derived/imu.hpp>
```

| Field | Type |
|-------|------|
| `header` | `Header` |
| `orientation` | `Orientation` |
| `angular_velocity` | `Vec3` |
| `linear_acceleration` | `Vec3` |

### Odometry

Full odometry with pose and velocities.

```cpp
#include <conduit_types/derived/odometry.hpp>
```

| Field | Type |
|-------|------|
| `header` | `Header` |
| `child_frame` | `char[64]` |
| `pose` | `Pose3D` |
| `linear_velocity` | `Vec3` |
| `angular_velocity` | `Vec3` |

```cpp
Odometry odom{};
set_frame(odom.header.frame, "odom");
set_frame(odom.child_frame, "base_link");
odom.pose.position.x = 1.0;
odom.pose.orientation = Orientation::from_yaw(0.5);
odom.linear_velocity.x = 1.0;
```

## Serialization Helpers

`WriteBuffer` and `ReadBuffer` simplify serialization for variable message types.

```cpp
#include <conduit_types/buffer.hpp>
```

### WriteBuffer

```cpp
// Calculate size
size_t size = WriteBuffer::size_of(my_string) + WriteBuffer::size_of(my_double);

// Serialize
WriteBuffer buf(buffer_ptr);
buf.write(my_string);   // writes length-prefixed string
buf.write(my_double);   // writes trivially copyable value
```

### ReadBuffer

```cpp
ReadBuffer buf(data_ptr, data_size);
auto s = buf.read<std::string>();   // reads length-prefixed string
auto d = buf.read<double>();        // reads trivially copyable value
```

### Example: Custom Variable Message

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

## Creating Custom Types

### Fixed Type

```cpp
#include <conduit_types/fixed_message_type.hpp>

struct MotorCommand : conduit::FixedMessageType {
    double velocity;
    double torque;
    uint32_t motor_id;
};

// Use it
Publisher<MotorCommand> pub("motor_cmd");
MotorCommand cmd{};
cmd.velocity = 1.5;
cmd.torque = 0.3;
cmd.motor_id = 1;
pub.publish(cmd);
```

### Composing Built-in Types

```cpp
#include <conduit_types/fixed_message_type.hpp>
#include <conduit_types/header.hpp>
#include <conduit_types/primitives/vec3.hpp>
#include <conduit_types/derived/orientation.hpp>

struct RobotState : conduit::FixedMessageType {
    conduit::Header header;
    conduit::Vec3 position;
    conduit::Orientation orientation;
    conduit::Vec3 velocity;
    double battery_voltage;
};
```
