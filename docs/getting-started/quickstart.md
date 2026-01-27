# Quick Start

This guide walks through creating a simple publisher and subscriber.

## Publisher

Create a node that publishes messages at a fixed rate:

```cpp
#include <conduit_core/node.hpp>
#include <conduit_core/log.hpp>
#include <optional>

class HelloPublisher : public conduit::Node {
public:
    HelloPublisher() {
        pub_.emplace(advertise("hello"));
        loop(1.0, &HelloPublisher::publish);  // 1 Hz
    }

private:
    void publish() {
        std::string msg = "Hello World #" + std::to_string(count_++);
        pub_->publish(msg.data(), msg.size());
        conduit::log::info("Published: {}", msg);
    }

    std::optional<conduit::Publisher> pub_;
    uint64_t count_ = 0;
};

int main() {
    HelloPublisher node;
    node.run();  // Blocks until Ctrl+C
    return 0;
}
```

## Subscriber

Create a node that receives and processes messages:

```cpp
#include <conduit_core/node.hpp>
#include <conduit_core/log.hpp>
#include <string>

class HelloSubscriber : public conduit::Node {
public:
    HelloSubscriber() {
        subscribe("hello", &HelloSubscriber::on_message);
    }

private:
    void on_message(const conduit::Message& msg) {
        std::string text(static_cast<const char*>(msg.data()), msg.size());
        conduit::log::info("Received: {}", text);
    }
};

int main() {
    HelloSubscriber node;
    node.run();
    return 0;
}
```

## Key Concepts

### Node

The `Node` class manages:

- **Subscriptions** - Each subscription runs in its own thread
- **Loops** - Timer callbacks at fixed rates
- **Signal handling** - Graceful shutdown on SIGINT/SIGTERM

### Publisher

```cpp
Publisher pub = advertise("topic_name");
pub.publish(data_ptr, data_size);
```

### Subscriber

```cpp
subscribe("topic_name", &MyNode::callback);

void callback(const Message& msg) {
    // msg.data() - pointer to payload
    // msg.size() - payload size in bytes
    // msg.sequence() - message sequence number
    // msg.timestamp_ns() - publish timestamp
}
```

### Loop

```cpp
loop(10.0, &MyNode::my_function);  // Call at 10 Hz
```

## Running

Build and run:

```bash
# Build
forge build conduit_demo

# Terminal 1: Start publisher
./install/bin/demo-publisher

# Terminal 2: Start subscriber
./install/bin/demo-subscriber
```

## Monitoring

Use CLI tools to inspect the system:

```bash
# List active topics
conduit topics

# Show topic info
conduit info hello

# Echo messages
conduit echo hello

# Measure publish rate
conduit hz hello

# Record to file
conduit record -o data.mcap hello
```
