# Getting Started

Get Conduit running in under 5 minutes.

## Prerequisites

- Docker
- Linux host (for shared memory)

## Setup

### 1. Clone and enter the repo

```bash
git clone https://github.com/venkisagunner93/conduit.git
cd conduit
```

### 2. Source the helper script

```bash
source conduit.sh
```

This adds shell commands:

| Command | Description |
|---------|-------------|
| `cbuild` | Build the Docker image |
| `crun` | Start the container |
| `cexec` | Exec into running container |
| `cforge` | Install the forge build tool |

### 3. Build and enter the container

```bash
cbuild    # Build Docker image (one-time)
crun      # Start container
```

You're now inside the container at `/home/docker_user/workspace`.

### 4. Install forge and build

```bash
cforge        # Install build tool
forge build   # Build all packages
```

### 5. Set up your environment

```bash
source install/setup.bash
```

This adds `install/bin` to your PATH.

## Run the Demo

Open three terminals (use `cexec` to attach to the running container).

**Terminal 1 — Publisher:**
```bash
demo-publisher
```

**Terminal 2 — Subscriber:**
```bash
demo-subscriber
```

**Terminal 3 — Monitor:**
```bash
conduit topics      # List topics
conduit hz hello    # Measure rate
conduit echo hello  # See messages
```

Or launch everything with a flow:

```bash
conduit flow demo
```

## Create Your First Node

### 1. Create a package

```bash
forge pkg my_robot --deps conduit_core conduit_types
```

This creates:
```
packages/my_robot/
├── conduit.toml
├── CMakeLists.txt
├── include/my_robot/my_robot.hpp
├── src/my_robot.cpp
└── tests/my_robot_test.cpp
```

### 2. Write a publisher

Edit `packages/my_robot/src/my_robot.cpp`:

```cpp
#include <conduit_core/node.hpp>
#include <conduit_core/log.hpp>
#include <conduit_types/primitives/uint.hpp>
#include <optional>

using namespace conduit;

class MyPublisher : public Node {
public:
    MyPublisher() {
        pub_.emplace(advertise<Uint>("my_topic"));
        loop(10.0, &MyPublisher::publish);
    }

private:
    void publish() {
        Uint msg{};
        msg.value = count_++;
        pub_->publish(msg);
        log::info("Sent: {}", msg.value);
    }

    std::optional<Publisher<Uint>> pub_;
    uint64_t count_ = 0;
};

int main() {
    MyPublisher node;
    node.run();
}
```

### 3. Write a subscriber

```cpp
#include <conduit_core/node.hpp>
#include <conduit_core/log.hpp>
#include <conduit_types/primitives/uint.hpp>

using namespace conduit;

class MySubscriber : public Node {
public:
    MySubscriber() {
        subscribe<Uint>("my_topic", &MySubscriber::on_message);
    }

private:
    void on_message(const TypedMessage<Uint>& msg) {
        log::info("Received [seq={}]: {}", msg.sequence, msg.data.value);
    }
};

int main() {
    MySubscriber node;
    node.run();
}
```

### 4. Update CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_robot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(conduit_core REQUIRED)
find_package(conduit_types REQUIRED)

add_executable(my_publisher src/my_robot.cpp)
target_link_libraries(my_publisher conduit_core::conduit_core)

install(TARGETS my_publisher RUNTIME DESTINATION bin)
```

### 5. Build and run

```bash
forge build my_robot
./install/bin/my_publisher
```

In another terminal:
```bash
conduit echo my_topic
```

## Project Structure

```
conduit/
├── packages/           # All Conduit packages
│   ├── conduit_core/   # Core library
│   ├── conduit_types/  # Message types
│   ├── conduit_tank/   # Recording
│   ├── conduit_flow/   # Flow execution
│   ├── conduit_tools/  # CLI
│   └── conduit_demo/   # Examples
├── tools/
│   └── conduit-forge/  # Build tool
├── install/            # Built binaries and libraries
├── build/              # Build artifacts
└── conduit.sh          # Helper script
```

## Build Commands

```bash
forge build              # Build all packages
forge build <package>    # Build specific package
forge test               # Run all tests
forge list               # Show packages and build order
forge clean              # Remove build artifacts
forge pkg <name>         # Create new package
```

## Next Steps

- [Publisher API](api/publisher.md) — How to publish typed messages
- [Subscriber API](api/subscriber.md) — How to subscribe to topics
- [Types](api/types.md) — Built-in message types
- [Loop API](api/loop.md) — How to run code at fixed rates
- [CLI Tools](cli.md) — Monitor, record, and launch
- [Architecture](architecture/index.md) — How it works under the hood
