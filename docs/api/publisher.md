# Publisher

The `Publisher` class writes messages to a topic.

## Overview

A Publisher:

- Creates shared memory for the topic
- Manages the ring buffer
- Wakes subscribers when new data arrives

## Basic Usage

```cpp
#include <conduit_core/pubsub.hpp>

// Create publisher
conduit::Publisher pub("my_topic");

// Publish data
std::string msg = "Hello";
pub.publish(msg.data(), msg.size());
```

## API Reference

### Constructor

```cpp
Publisher(const std::string& topic);
Publisher(const std::string& topic, const RingBufferConfig& config);
```

Creates a publisher for the given topic.

**Parameters:**

- `topic` - Topic name (creates `/dev/shm/conduit_{topic}`)
- `config` - Optional ring buffer configuration

**Example:**
```cpp
// Default config (16 slots, 4KB each)
Publisher pub("sensor_data");

// Custom config for large messages
RingBufferConfig config;
config.slot_count = 4;
config.slot_size = 10'000'000;  // 10MB
Publisher large_pub("camera", config);
```

!!! warning "Single Publisher"
    Only one publisher can exist per topic. Creating a second publisher
    for the same topic throws an exception.

### publish()

```cpp
bool publish(const void* data, size_t size);
```

Publishes a message to the topic.

**Parameters:**

- `data` - Pointer to message data
- `size` - Size in bytes

**Returns:** `true` if published, `false` if message too large for slot

**Example:**
```cpp
struct SensorData {
    double timestamp;
    float values[100];
};

SensorData data;
// ... fill data ...
pub.publish(&data, sizeof(data));
```

### topic()

```cpp
const std::string& topic() const;
```

Returns the topic name.

## With Node

When using the Node class, create publishers via `advertise()`:

```cpp
class MyNode : public conduit::Node {
public:
    MyNode() {
        // Use std::optional because Publisher has no default constructor
        pub_.emplace(advertise("output"));
    }

private:
    void send() {
        pub_->publish(data, size);
    }

    std::optional<conduit::Publisher> pub_;
};
```

## Configuration

### RingBufferConfig

| Field | Default | Description |
|-------|---------|-------------|
| `slot_count` | 16 | Number of message slots (power of 2) |
| `slot_size` | 4096 | Bytes per slot (header + payload) |

**Choosing slot_count:**

- More slots = more history, more memory
- Fewer slots = less memory, risk of overwriting slow readers
- Typical: 4-64 slots

**Choosing slot_size:**

- Must fit largest message + 20 byte header
- Round up to accommodate peak sizes
- Example: 1MB images need `slot_size >= 1'000'020`

## Message Framing

Each published message includes automatic framing:

| Field | Type | Description |
|-------|------|-------------|
| `size` | uint32_t | Payload size |
| `sequence` | uint64_t | Monotonic counter |
| `timestamp_ns` | uint64_t | CLOCK_MONOTONIC_RAW |
| `payload` | bytes | Your data |

Subscribers receive this metadata via the `Message` class.

## Performance Tips

1. **Reuse buffers** - Avoid allocating for each publish
2. **Right-size slots** - Don't waste memory on oversized slots
3. **Batch small messages** - Combine related data to reduce overhead
