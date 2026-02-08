# Conduit

**Zero-copy pub/sub middleware for robotics.**

Conduit is a lightweight IPC framework built for robotics engineers who want to focus on robotics, not fight their middleware.

## Features

| Feature | Description |
|---------|-------------|
| **Zero-copy transport** | Messages written once to shared memory, read directly by subscribers |
| **Lock-free** | No mutexes, no deadlocks, crashed processes don't block others |
| **Zero CPU when idle** | Futex-based signaling — sleeping subscribers use no CPU |
| **Typed messages** | Compile-time validated `Publisher<T>` / `Subscriber<T>` with built-in types |
| **Simple API** | Node class handles threading, signal handling, and callbacks |
| **YAML flow files** | Readable launch files that do exactly what they say |
| **Built-in recording** | MCAP format with Zstd/LZ4 compression |
| **CLI tools** | Monitor topics, measure rates, record data |

## Quick Look

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/imu.hpp>
#include <conduit_types/primitives/vec3.hpp>

using namespace conduit;

class MyNode : public Node {
public:
    MyNode() {
        subscribe<Imu>("imu", &MyNode::on_imu);
        pub_.emplace(advertise<Vec3>("output"));
        loop(100.0, &MyNode::control);  // 100 Hz
    }

private:
    void on_imu(const TypedMessage<Imu>& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_accel_ = msg.data.linear_acceleration;
    }

    void control() {
        Vec3 cmd{};
        {
            std::lock_guard<std::mutex> lock(mutex_);
            cmd = latest_accel_;
        }
        pub_->publish(cmd);
    }

    std::mutex mutex_;
    Vec3 latest_accel_{};
    std::optional<Publisher<Vec3>> pub_;
};

int main() {
    MyNode node;
    node.run();  // Blocks until Ctrl+C
}
```

## Performance

| Operation | Latency |
|-----------|---------|
| Publish (small message) | ~200-500 ns |
| Publish (1 MB) | ~100-200 us |
| Subscriber wake | ~2-5 us |

## Packages

| Package | Description |
|---------|-------------|
| `conduit_core` | Shared memory, ring buffer, pub/sub, node |
| `conduit_types` | Message types: primitives, derived composites, serialization helpers |
| `conduit_tank` | MCAP recording with compression |
| `conduit_flow` | Flow file execution engine |
| `conduit_tools` | CLI (topics, echo, hz, record, flow) |
| `conduit_demo` | Example publisher and subscriber |
