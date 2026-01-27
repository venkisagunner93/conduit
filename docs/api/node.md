# Node

The `Node` class is the main abstraction for building Conduit applications.

## Overview

A Node manages:

- **Subscriptions** - Each runs in its own thread
- **Loops** - Timer callbacks at fixed rates
- **Signal handling** - Graceful shutdown on SIGINT/SIGTERM
- **Logging** - Built-in logging with topic prefix

## Basic Usage

```cpp
#include <conduit_core/node.hpp>

class MyNode : public conduit::Node {
public:
    MyNode() {
        // Setup in constructor
        subscribe("input", &MyNode::on_input);
        pub_ = advertise("output");
        loop(10.0, &MyNode::process);
    }

private:
    void on_input(const conduit::Message& msg) {
        // Handle incoming message
    }

    void process() {
        // Called at 10 Hz
        pub_.publish(data, size);
    }

    conduit::Publisher pub_;
};

int main() {
    MyNode node;
    node.run();  // Blocks until SIGINT/SIGTERM
    return 0;
}
```

## API Reference

### Constructor

```cpp
Node();
```

Creates a node. Setup subscriptions, publishers, and loops in derived class constructor.

### run()

```cpp
void run();
```

Starts all subscription threads and loop threads. Blocks until:

- `SIGINT` received (Ctrl+C)
- `SIGTERM` received
- `stop()` called from another thread

Example:
```cpp
node.run();  // Blocks here
// After run() returns, all threads have stopped
```

### stop()

```cpp
void stop();
```

Signals all threads to stop. Can be called from any thread.

### subscribe()

```cpp
template<typename T, typename Func>
void subscribe(const std::string& topic, Func T::* callback);
```

Subscribe to a topic. The callback runs in a dedicated thread.

**Parameters:**

- `topic` - Topic name to subscribe to
- `callback` - Member function pointer: `void(const Message&)`

**Example:**
```cpp
subscribe("imu", &MyNode::on_imu);

void on_imu(const conduit::Message& msg) {
    // Called for each message on "imu" topic
}
```

### advertise()

```cpp
Publisher advertise(const std::string& topic);
Publisher advertise(const std::string& topic, const RingBufferConfig& config);
```

Create a publisher for a topic.

**Parameters:**

- `topic` - Topic name to publish to
- `config` - Optional ring buffer configuration

**Example:**
```cpp
auto pub = advertise("output");
auto large_pub = advertise("images", {.slot_count = 4, .slot_size = 5'000'000});
```

### loop()

```cpp
template<typename T, typename Func>
void loop(double rate_hz, Func T::* callback);

void loop(double rate_hz, std::function<void()> callback);
```

Register a timer callback at a fixed rate.

**Parameters:**

- `rate_hz` - Frequency in Hz (e.g., 10.0 for 10 Hz)
- `callback` - Function to call

**Example:**
```cpp
loop(100.0, &MyNode::control_loop);  // 100 Hz

void control_loop() {
    // Called every 10ms
}
```

!!! warning "Deadline Misses"
    If the callback takes longer than the period, a warning is logged and
    the next iteration runs immediately.

## Threading Model

```
┌─────────────────────────────────────────────────────────────┐
│                        Main Thread                          │
│                                                             │
│   node.run() blocks here, handles signals                   │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────┐  ┌─────────────────────┐
│ Subscription Thread │  │ Subscription Thread │
│                     │  │                     │
│ subscriber.wait()   │  │ subscriber.wait()   │
│ callback()          │  │ callback()          │
│ repeat...           │  │ repeat...           │
└─────────────────────┘  └─────────────────────┘

┌─────────────────────┐  ┌─────────────────────┐
│    Loop Thread      │  │    Loop Thread      │
│                     │  │                     │
│ sleep_until(next)   │  │ sleep_until(next)   │
│ callback()          │  │ callback()          │
│ repeat...           │  │ repeat...           │
└─────────────────────┘  └─────────────────────┘
```

Each subscription and loop runs in its own thread. Callbacks may run concurrently.

## Signal Handling

The node installs handlers for SIGINT and SIGTERM:

```
Ctrl+C → SIGINT → stop() → all threads exit → run() returns
```

Cleanup code runs after `run()` returns:

```cpp
int main() {
    MyNode node;
    node.run();
    // Cleanup here - all threads have stopped
    return 0;
}
```

## Logging

Use the built-in logging functions:

```cpp
#include <conduit_core/log.hpp>

conduit::log::info("Message: {}", value);
conduit::log::warn("Warning: {}", reason);
conduit::log::error("Error: {}", error);
conduit::log::debug("Debug: {}", details);
```

Logging uses [fmt](https://fmt.dev) format strings.
