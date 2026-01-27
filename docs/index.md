# Conduit

**Zero-copy pub/sub middleware for robotics.**

Conduit is a lightweight IPC framework built for robotics engineers who want to focus on robotics, not fight their middleware.

## Features

| Feature | Description |
|---------|-------------|
| **Zero-copy transport** | Messages written once to shared memory, read directly by subscribers |
| **Lock-free** | No mutexes, no deadlocks, crashed processes don't block others |
| **Zero CPU when idle** | Futex-based signaling — sleeping subscribers use no CPU |
| **Simple API** | Node class handles threading, signal handling, and callbacks |
| **YAML flow files** | Readable launch files that do exactly what they say |
| **Built-in recording** | MCAP format with Zstd/LZ4 compression |
| **CLI tools** | Monitor topics, measure rates, record data |

## Quick Look

```cpp
class MyNode : public conduit::Node {
public:
    MyNode() {
        subscribe("imu", &MyNode::on_imu);
        pub_.emplace(advertise("output"));
        loop(100.0, &MyNode::control);  // 100 Hz
    }

private:
    void on_imu(const conduit::Message& msg) {
        // Handle IMU data
    }

    void control() {
        // Runs at 100 Hz
        pub_->publish(cmd, sizeof(cmd));
    }

    std::optional<conduit::Publisher> pub_;
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
| Publish (1 MB) | ~100-200 µs |
| Subscriber wake | ~2-5 µs |

## Packages

| Package | Description |
|---------|-------------|
| `conduit_core` | Shared memory, ring buffer, pub/sub, node |
| `conduit_tank` | MCAP recording with compression |
| `conduit_flow` | Flow file execution engine |
| `conduit_tools` | CLI (topics, echo, hz, record, flow) |
| `conduit_demo` | Example publisher and subscriber |
