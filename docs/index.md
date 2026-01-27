# Conduit

**Zero-copy pub/sub middleware for robotics.**

Conduit provides high-performance inter-process communication using shared memory, designed for robotics applications where latency and throughput matter.

## Features

- **Zero-copy transport** - Messages are written once to shared memory and read directly by subscribers
- **Lock-free design** - No mutexes, no deadlocks, crashed processes don't block others
- **Zero CPU when idle** - Futex-based signaling means sleeping subscribers use no CPU
- **Simple API** - Node abstraction handles threading and callbacks automatically
- **Built-in recording** - MCAP format with Zstd/LZ4 compression
- **CLI tools** - Monitor topics, measure rates, record data

## Quick Example

```cpp
#include <conduit_core/node.hpp>

class MyNode : public conduit::Node {
public:
    MyNode() {
        // Subscribe to IMU data
        subscribe("imu", &MyNode::on_imu);

        // Publish at 10 Hz
        pub_ = advertise("processed");
        loop(10.0, &MyNode::process);
    }

private:
    void on_imu(const conduit::Message& msg) {
        // Handle incoming IMU data
    }

    void process() {
        // Publish processed data
        pub_.publish(data, size);
    }

    conduit::Publisher pub_;
};

int main() {
    MyNode node;
    node.run();  // Blocks until SIGINT/SIGTERM
}
```

## Performance

| Operation | Latency |
|-----------|---------|
| Publish small message | ~200-500 ns |
| Publish 1MB message | ~100-200 µs |
| Subscribe wake from sleep | ~2-5 µs |

## Getting Started

1. [Installation](getting-started/installation.md) - Set up the development environment
2. [Quick Start](getting-started/quickstart.md) - Build and run the demo

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         User Code                           │
│   class MyNode : public conduit::Node { ... }               │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      conduit::Node                          │
│   - Thread per subscription                                 │
│   - Callbacks dispatched automatically                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Publisher / Subscriber                     │
│   - Publisher: write + futex wake                           │
│   - Subscriber: futex wait + read                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    SPMC Ring Buffer                         │
│   - Lock-free, cache-line aligned                           │
│   - Sequence validation for overwrite detection             │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                /dev/shm/conduit_{topic}                     │
└─────────────────────────────────────────────────────────────┘
```
