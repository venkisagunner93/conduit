# Loop

Run code at a fixed rate.

## Basic Usage

```cpp
#include <conduit_core/node.hpp>

class MyNode : public conduit::Node {
public:
    MyNode() {
        loop(100.0, &MyNode::control);  // 100 Hz
    }

private:
    void control() {
        // Called every 10 ms
    }
};
```

## API

### loop()

```cpp
template<typename T, typename Func>
void loop(double rate_hz, Func T::* callback);

void loop(double rate_hz, std::function<void()> callback);
```

Register a function to be called at a fixed rate.

| Parameter | Description |
|-----------|-------------|
| `rate_hz` | Frequency in Hz (e.g., 100.0 for 100 Hz) |
| `callback` | Member function or `std::function<void()>` |

The loop runs in a dedicated thread. Call `loop()` in your constructor, before `run()`.

## How It Works

```cpp
// Internally, each loop thread does:
auto next_time = now();
while (running) {
    next_time += period;
    callback();
    sleep_until(next_time);
}
```

Key behaviors:
- Uses `std::chrono::steady_clock` for timing
- Sleeps precisely to hit target rate
- If callback overruns, next iteration runs immediately

## Deadline Handling

If your callback takes longer than the period:

```
Period: 10 ms (100 Hz)
Callback takes: 15 ms

Iteration 1: callback() runs, takes 15 ms
             -> Missed deadline by 5 ms
             -> Warning logged
             -> Iteration 2 starts immediately

Iteration 2: callback() runs, takes 8 ms
             -> On time, sleeps 2 ms until next
```

A warning is logged when deadlines are missed.

## Examples

### Control Loop

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/primitives/vec3.hpp>
#include <conduit_types/derived/pose3d.hpp>

using namespace conduit;

class ControlNode : public Node {
public:
    ControlNode() {
        subscribe<Pose3D>("state", &ControlNode::on_state);
        pub_.emplace(advertise<Vec3>("cmd"));
        loop(100.0, &ControlNode::control);  // 100 Hz control
    }

private:
    void on_state(const TypedMessage<Pose3D>& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_state_ = msg.data;
    }

    void control() {
        Pose3D state;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state = latest_state_;
        }

        Vec3 cmd{};
        cmd.x = compute_control(state);
        pub_->publish(cmd);
    }

    std::mutex mutex_;
    Pose3D latest_state_{};
    std::optional<Publisher<Vec3>> pub_;
};
```

### Multiple Loops

```cpp
class MultiRateNode : public conduit::Node {
public:
    MultiRateNode() {
        loop(1000.0, &MultiRateNode::fast_loop);   // 1 kHz
        loop(100.0, &MultiRateNode::medium_loop);  // 100 Hz
        loop(10.0, &MultiRateNode::slow_loop);     // 10 Hz
    }

private:
    void fast_loop() {
        // Sensor reading, 1 kHz
    }

    void medium_loop() {
        // Control, 100 Hz
    }

    void slow_loop() {
        // Logging/telemetry, 10 Hz
    }
};
```

### Publish at Fixed Rate

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/primitives/uint.hpp>

using namespace conduit;

class HeartbeatNode : public Node {
public:
    HeartbeatNode() {
        pub_.emplace(advertise<Uint>("heartbeat"));
        loop(1.0, &HeartbeatNode::heartbeat);  // 1 Hz
    }

private:
    void heartbeat() {
        Uint msg{};
        msg.value = count_++;
        pub_->publish(msg);
    }

    std::optional<Publisher<Uint>> pub_;
    uint64_t count_ = 0;
};
```

### Combining Subscribe and Loop

Common pattern: subscribe for input, loop for output.

```cpp
#include <conduit_core/node.hpp>
#include <conduit_types/derived/imu.hpp>
#include <conduit_types/primitives/vec3.hpp>

using namespace conduit;

class ProcessorNode : public Node {
public:
    ProcessorNode() {
        subscribe<Imu>("imu", &ProcessorNode::on_input);
        pub_.emplace(advertise<Vec3>("filtered_accel"));
        loop(50.0, &ProcessorNode::process);  // 50 Hz output
    }

private:
    void on_input(const TypedMessage<Imu>& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_ = msg.data;
        has_data_ = true;
    }

    void process() {
        Imu input;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!has_data_) return;
            input = latest_;
        }

        Vec3 filtered{};
        filtered.x = filter(input.linear_acceleration.x);
        filtered.y = filter(input.linear_acceleration.y);
        filtered.z = filter(input.linear_acceleration.z);
        pub_->publish(filtered);
    }

    std::mutex mutex_;
    Imu latest_{};
    bool has_data_ = false;
    std::optional<Publisher<Vec3>> pub_;
};
```

## Common Rates

| Use Case | Typical Rate |
|----------|--------------|
| IMU reading | 200-1000 Hz |
| Motor control | 100-1000 Hz |
| Path planning | 10-50 Hz |
| Localization | 10-100 Hz |
| Telemetry/logging | 1-10 Hz |
| Watchdog/heartbeat | 1 Hz |

## Thread Safety

Each loop runs in its own thread. If you share data between:
- Multiple loops
- Loops and subscriptions

**Use a mutex:**

```cpp
std::mutex mutex_;
Vec3 shared_{};

void on_input(const TypedMessage<Vec3>& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    shared_ = msg.data;
}

void loop_callback() {
    std::lock_guard<std::mutex> lock(mutex_);
    process(shared_);
}
```
