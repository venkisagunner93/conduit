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

**Terminal 1 — Subscriber:**
```bash
demo-subscriber
```

**Terminal 2 — Publisher:**
```bash
demo-publisher
```

**Terminal 3 — Monitor:**
```bash
conduit topics      # List topics
conduit hz hello    # Measure rate
conduit echo hello  # See messages
```

## Create Your First Node

### 1. Create a package

```bash
forge pkg my_robot --deps conduit_core
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
#include <optional>

class MyPublisher : public conduit::Node {
public:
    MyPublisher() {
        pub_.emplace(advertise("my_topic"));
        loop(10.0, &MyPublisher::publish);
    }

private:
    void publish() {
        std::string msg = "Hello " + std::to_string(count_++);
        pub_->publish(msg.data(), msg.size());
        conduit::log::info("Sent: {}", msg);
    }

    std::optional<conduit::Publisher> pub_;
    uint64_t count_ = 0;
};

int main() {
    MyPublisher node;
    node.run();
}
```

### 3. Update CMakeLists.txt

Replace the library with an executable:

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_robot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(conduit_core REQUIRED)

add_executable(my_publisher src/my_robot.cpp)
target_link_libraries(my_publisher conduit_core::conduit_core)

install(TARGETS my_publisher RUNTIME DESTINATION bin)
```

### 4. Build and run

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

- [Publisher API](api/publisher.md) — How to publish messages
- [Subscriber API](api/subscriber.md) — How to subscribe to topics
- [Loop API](api/loop.md) — How to run code at fixed rates
- [CLI Tools](cli.md) — Monitor and record
- [Architecture](architecture.md) — How it works under the hood
