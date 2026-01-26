# Conduit

Zero-copy pub/sub middleware for robotics.

## Features

- **Zero-copy** - Shared memory transport
- **Lock-free** - SPMC ring buffer
- **Zero CPU idle** - Futex signaling
- **Recording** - MCAP with Zstd/LZ4
- **Simple API** - Node with callbacks and timers

## Getting Started

```bash
# Source helper functions
source conduit.sh

# Build and run container
cbuild            # Build Docker image
crun              # Run container (interactive)

# Inside container
cforge            # Install forge build tool
source install/setup.bash
forge build       # Build all packages
```

## Docker Commands

| Command | Description |
|---------|-------------|
| `cbuild` | Build Docker image from Dockerfile |
| `crun` | Run container with workspace mounted |
| `cexec` | Exec into running container |
| `cforge` | Install forge build tool (inside container) |

## Build

```bash
forge build      # Build all
forge test       # Run tests
forge clean      # Clean
```

## Demo

```bash
# Terminal 1 - subscriber waits for publisher
demo-subscriber

# Terminal 2
demo-publisher
```

## Node API

```cpp
#include <conduit_core/node.hpp>

class MyNode : public conduit::Node {
public:
    MyNode() {
        subscribe("input", &MyNode::on_input);
        pub_.emplace(advertise("output"));
        loop(10.0, &MyNode::update);  // 10 Hz
    }

private:
    void on_input(const conduit::Message& msg) {
        // handle message
    }

    void update() {
        pub_->publish(data, size);
    }

    std::optional<conduit::Publisher> pub_;
};

int main() {
    MyNode node;
    node.run();  // blocks until Ctrl+C
}
```

## CLI

```bash
conduit topics                        # list topics
conduit info <topic>                  # topic details
conduit echo <topic>                  # print messages
conduit hz <topic>                    # measure rate
conduit record -o out.mcap <topics>   # record
conduit launch config.yaml            # launch nodes
```

## Packages

| Package | Description |
|---------|-------------|
| conduit_core | pub/sub, node, ring buffer |
| conduit_tank | MCAP recording |
| conduit_tools | CLI |
| conduit_demo | examples |

## Requirements

- Linux
- C++17
- CMake 3.16+
