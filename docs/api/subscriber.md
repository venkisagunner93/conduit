# Subscriber

Receive typed messages from a topic.

## Basic Usage

```cpp
#include <conduit_core/subscriber.hpp>
#include <conduit_types/primitives/vec3.hpp>

conduit::Subscriber<conduit::Vec3> sub("imu_accel");

while (running) {
    auto msg = sub.wait();
    process(msg.data);  // msg.data is a Vec3
}
```

## With Node

The preferred way — Node handles threading:

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/imu.hpp>

using namespace conduit;

class MyNode : public Node {
public:
    MyNode() {
        subscribe<Imu>("imu", &MyNode::on_imu);
    }

private:
    void on_imu(const TypedMessage<Imu>& msg) {
        // Called in a dedicated thread for each message
        log::info("Accel: [{}, {}, {}]",
            msg.data.linear_acceleration.x,
            msg.data.linear_acceleration.y,
            msg.data.linear_acceleration.z);
    }
};
```

Each subscription runs in its own thread. You don't manage the loop.

## API

### Constructor

```cpp
Subscriber<T>(const std::string& topic, const SubscriberOptions& options = {});
```

Opens shared memory at `/dev/shm/conduit_{topic}`.

If the topic doesn't exist yet, the constructor **waits** until the publisher creates it.

### wait()

```cpp
TypedMessage<T> wait();
```

Blocks until the next message arrives. Returns a `TypedMessage<T>` with the deserialized data.

This uses futex for efficient sleeping — zero CPU while waiting.

### take()

```cpp
std::optional<TypedMessage<T>> take();
```

Non-blocking read.

**Returns:** `TypedMessage<T>` if available, `std::nullopt` otherwise.

```cpp
if (auto msg = sub.take()) {
    process(msg->data);
}
```

### wait_for()

```cpp
std::optional<TypedMessage<T>> wait_for(std::chrono::nanoseconds timeout);
```

Blocks until a message arrives or timeout.

```cpp
using namespace std::chrono_literals;

if (auto msg = sub.wait_for(100ms)) {
    process(msg->data);
} else {
    // Timed out
}
```

### topic()

```cpp
const std::string& topic() const;
```

Returns the topic name.

## TypedMessage

```cpp
template <typename T>
struct TypedMessage {
    T data;                 // The deserialized message
    uint64_t sequence;      // Message sequence number
    uint64_t timestamp_ns;  // Publish timestamp (nanoseconds)
};
```

Access the message data directly:

```cpp
auto msg = sub.wait();
double x = msg.data.x;          // Access fields directly
uint64_t seq = msg.sequence;     // Sequence number
uint64_t ts = msg.timestamp_ns;  // Timestamp
```

## Node Subscribe

### Typed Subscribe

```cpp
subscribe<MsgType>("topic", &MyNode::callback);
```

The callback receives `const TypedMessage<MsgType>&`:

```cpp
void on_imu(const TypedMessage<Imu>& msg) {
    Imu imu = msg.data;
    uint64_t seq = msg.sequence;
}
```

### Raw Subscribe

For tools and introspection, raw subscribe is still available:

```cpp
subscribe("topic", &MyNode::callback);
```

The callback receives `const conduit::Message&` with raw bytes:

```cpp
void on_raw(const conduit::Message& msg) {
    const void* data = msg.data;
    size_t size = msg.size;
}
```

## Examples

### Typed Subscriber

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/odometry.hpp>

using namespace conduit;

class OdomSubscriber : public Node {
public:
    OdomSubscriber() {
        subscribe<Odometry>("odom", &OdomSubscriber::on_odom);
    }

private:
    void on_odom(const TypedMessage<Odometry>& msg) {
        const auto& odom = msg.data;
        log::info("Position: [{}, {}, {}] yaw: {}",
            odom.pose.position.x,
            odom.pose.position.y,
            odom.pose.position.z,
            odom.pose.orientation.to_yaw());
    }
};
```

### Multiple Subscriptions

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/imu.hpp>
#include <conduit_types/derived/pose3d.hpp>

using namespace conduit;

class FusionNode : public Node {
public:
    FusionNode() {
        subscribe<Imu>("imu", &FusionNode::on_imu);
        subscribe<Pose3D>("gps_pose", &FusionNode::on_gps);
        // Each runs in its own thread
    }

private:
    void on_imu(const TypedMessage<Imu>& msg) { /* ... */ }
    void on_gps(const TypedMessage<Pose3D>& msg) { /* ... */ }
};
```

### Manual Loop (Without Node)

```cpp
#include <conduit_core/subscriber.hpp>
#include <conduit_types/primitives/vec3.hpp>
#include <atomic>
#include <signal.h>

std::atomic<bool> running{true};

void signal_handler(int) { running = false; }

int main() {
    signal(SIGINT, signal_handler);

    conduit::Subscriber<conduit::Vec3> sub("accel");

    while (running) {
        if (auto msg = sub.wait_for(std::chrono::milliseconds(100))) {
            process(msg->data);
        }
    }
}
```

## Multiple Subscribers Per Topic

Multiple processes/threads can subscribe to the same topic:

```
Publisher → topic "imu" → Subscriber A (Process 1)
                       → Subscriber B (Process 1)
                       → Subscriber C (Process 2)
```

Each subscriber:
- Gets **all** messages (no load balancing)
- Progresses independently
- Has its own read position

Maximum 16 subscribers per topic.

## Slow Subscriber Handling

If a subscriber can't keep up, the publisher eventually overwrites unread data:

```
write_idx = 100
read_idx  = 80   <- Subscriber is 20 messages behind
slot_count = 16

100 - 80 = 20 > 16  <- Subscriber has fallen behind!
```

When this happens:
1. Subscriber detects via sequence number mismatch
2. Subscriber skips to newest available data
3. Warning can be logged

**Solutions:**
- Increase `depth` in `PublisherOptions` for more buffer space
- Make callback faster
- Accept occasional drops (common for sensors)
