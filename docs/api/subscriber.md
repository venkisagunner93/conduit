# Subscriber

Receive messages from a topic.

## Basic Usage

```cpp
#include <conduit_core/pubsub.hpp>

conduit::Subscriber sub("my_topic");

while (running) {
    conduit::Message msg = sub.wait();
    process(msg.data(), msg.size());
}
```

## With Node

The preferred way — Node handles threading:

```cpp
#include <conduit_core/node.hpp>

class MyNode : public conduit::Node {
public:
    MyNode() {
        subscribe("input", &MyNode::on_message);
    }

private:
    void on_message(const conduit::Message& msg) {
        // Called in a dedicated thread for each message
        const MyStruct* data = static_cast<const MyStruct*>(msg.data());
        process(data);
    }
};
```

Each subscription runs in its own thread. You don't manage the loop.

## API

### Constructor

```cpp
Subscriber(const std::string& topic);
```

Opens shared memory at `/dev/shm/conduit_{topic}`.

If the topic doesn't exist yet, the constructor **waits** until the publisher creates it.

### wait()

```cpp
Message wait();
```

Blocks until the next message arrives.

This uses futex for efficient sleeping — zero CPU while waiting.

### try_read()

```cpp
std::optional<Message> try_read();
```

Non-blocking read.

**Returns:** Message if available, `std::nullopt` otherwise.

```cpp
if (auto msg = sub.try_read()) {
    process(msg->data(), msg->size());
}
```

### wait_for()

```cpp
std::optional<Message> wait_for(std::chrono::nanoseconds timeout);
```

Blocks until a message arrives or timeout.

```cpp
using namespace std::chrono_literals;

if (auto msg = sub.wait_for(100ms)) {
    process(msg->data(), msg->size());
} else {
    // Timed out
}
```

### topic()

```cpp
const std::string& topic() const;
```

Returns the topic name.

## Message Class

```cpp
class Message {
public:
    const void* data() const;       // Pointer to payload
    size_t size() const;            // Payload size in bytes
    uint64_t sequence() const;      // Message sequence number
    uint64_t timestamp_ns() const;  // Publish timestamp
};
```

### Zero-Copy Access

`data()` returns a pointer **directly into shared memory**. No copying.

```cpp
Message msg = sub.wait();
const ImuData* imu = static_cast<const ImuData*>(msg.data());
float ax = imu->accel[0];  // Reading shared memory directly
```

**Important:** The pointer is only valid until the next `wait()` / `try_read()`. Copy if you need to keep it.

### Measuring Latency

```cpp
#include <conduit_core/internal/time.hpp>

Message msg = sub.wait();
uint64_t now = conduit::internal::get_timestamp_ns();
uint64_t latency_ns = now - msg.timestamp_ns();
```

## Examples

### Simple Subscriber

```cpp
class EchoSubscriber : public conduit::Node {
public:
    EchoSubscriber() {
        subscribe("messages", &EchoSubscriber::on_message);
    }

private:
    void on_message(const conduit::Message& msg) {
        std::string text(static_cast<const char*>(msg.data()), msg.size());
        conduit::log::info("Received: {}", text);
    }
};
```

### Struct Subscriber

```cpp
struct ImuData {
    double timestamp;
    float accel[3];
    float gyro[3];
};

class ImuSubscriber : public conduit::Node {
public:
    ImuSubscriber() {
        subscribe("imu", &ImuSubscriber::on_imu);
    }

private:
    void on_imu(const conduit::Message& msg) {
        const ImuData* imu = static_cast<const ImuData*>(msg.data());
        conduit::log::info("Accel: [{}, {}, {}]",
            imu->accel[0], imu->accel[1], imu->accel[2]);
    }
};
```

### Multiple Subscriptions

```cpp
class FusionNode : public conduit::Node {
public:
    FusionNode() {
        subscribe("imu", &FusionNode::on_imu);
        subscribe("gps", &FusionNode::on_gps);
        subscribe("camera", &FusionNode::on_camera);
        // Each runs in its own thread
    }

private:
    void on_imu(const conduit::Message& msg) { /* ... */ }
    void on_gps(const conduit::Message& msg) { /* ... */ }
    void on_camera(const conduit::Message& msg) { /* ... */ }
};
```

### Manual Loop (Without Node)

```cpp
#include <conduit_core/pubsub.hpp>
#include <atomic>
#include <signal.h>

std::atomic<bool> running{true};

void signal_handler(int) { running = false; }

int main() {
    signal(SIGINT, signal_handler);

    conduit::Subscriber sub("my_topic");

    while (running) {
        if (auto msg = sub.wait_for(std::chrono::milliseconds(100))) {
            process(msg->data(), msg->size());
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
read_idx  = 80   ← Subscriber is 20 messages behind
slot_count = 16

100 - 80 = 20 > 16  ← Subscriber has fallen behind!
```

When this happens:
1. Subscriber detects via sequence number mismatch
2. Subscriber skips to newest available data
3. Warning can be logged

**Solutions:**
- Increase `slot_count` for more buffer space
- Make callback faster
- Accept occasional drops (common for sensors)
