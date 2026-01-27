# Subscriber

The `Subscriber` class reads messages from a topic.

## Overview

A Subscriber:

- Opens shared memory for the topic
- Claims a reader slot in the ring buffer
- Provides blocking and non-blocking read methods

## Basic Usage

```cpp
#include <conduit_core/pubsub.hpp>

// Create subscriber
conduit::Subscriber sub("my_topic");

// Wait for and read messages
while (running) {
    conduit::Message msg = sub.wait();
    process(msg.data(), msg.size());
}
```

## API Reference

### Constructor

```cpp
Subscriber(const std::string& topic);
```

Creates a subscriber for the given topic.

**Parameters:**

- `topic` - Topic name to subscribe to

The constructor waits for the topic to exist if it doesn't yet.

!!! info "Wait for Topic"
    If the publisher hasn't started, the constructor blocks until the topic
    appears. Use `ShmRegion::exists()` to check first if needed.

### wait()

```cpp
Message wait();
```

Waits for and returns the next message. Blocks until data is available.

**Returns:** The next message

**Example:**
```cpp
while (running) {
    Message msg = sub.wait();
    // Process msg.data(), msg.size()
}
```

This is the preferred method for most use cases. It uses futex for efficient sleeping (zero CPU when no data).

### try_read()

```cpp
std::optional<Message> try_read();
```

Attempts to read without blocking.

**Returns:** Message if available, `std::nullopt` if no new data

**Example:**
```cpp
if (auto msg = sub.try_read()) {
    process(msg->data(), msg->size());
} else {
    // No data available
}
```

### wait_for()

```cpp
std::optional<Message> wait_for(std::chrono::nanoseconds timeout);
```

Waits for a message with timeout.

**Parameters:**

- `timeout` - Maximum time to wait

**Returns:** Message if received, `std::nullopt` if timeout

**Example:**
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

The `Message` class provides access to received data:

```cpp
class Message {
public:
    const void* data() const;      // Pointer to payload
    size_t size() const;           // Payload size in bytes
    uint64_t sequence() const;     // Message sequence number
    uint64_t timestamp_ns() const; // Publish timestamp (nanoseconds)
};
```

### Zero-Copy Access

`data()` returns a pointer **directly into shared memory**. No copying occurs.

```cpp
Message msg = sub.wait();

// This pointer is valid until the next wait()/try_read()
const float* values = static_cast<const float*>(msg.data());
```

!!! warning "Pointer Lifetime"
    The pointer from `data()` is only valid until the next read operation.
    Copy the data if you need to keep it longer.

### Measuring Latency

Use the timestamp to measure end-to-end latency:

```cpp
Message msg = sub.wait();
uint64_t now = conduit::internal::get_timestamp_ns();
uint64_t latency_ns = now - msg.timestamp_ns();
```

## With Node

When using the Node class, use `subscribe()`:

```cpp
class MyNode : public conduit::Node {
public:
    MyNode() {
        subscribe("input", &MyNode::on_input);
    }

private:
    void on_input(const conduit::Message& msg) {
        // Called in dedicated thread for each message
        process(msg.data(), msg.size());
    }
};
```

The Node handles threading automatically - each subscription runs in its own thread.

## Multiple Subscribers

Multiple subscribers can read from the same topic:

```cpp
// Process A
Subscriber sub1("sensor");

// Process B
Subscriber sub2("sensor");

// Both receive all messages independently
```

Each subscriber:

- Claims its own slot (0-15)
- Tracks its own read position
- Receives all messages (no load balancing)

Maximum 16 subscribers per topic.

## Handling Slow Subscribers

If a subscriber falls behind, the publisher may overwrite unread data:

```
Publisher writes faster than subscriber reads
→ Publisher wraps around the ring buffer
→ Old data overwritten
→ Subscriber detects via sequence mismatch
→ Subscriber skips to newest available data
```

To handle this:

1. **Increase slot_count** - More buffer space
2. **Process faster** - Optimize callback
3. **Accept skipping** - Design for occasional drops
