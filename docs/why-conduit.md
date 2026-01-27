# Why Conduit?

Conduit exists because ROS 2 kept getting in the way.

## The Problem with ROS 2

If you've built robots with ROS 2, you've probably experienced:

**Mysterious latency spikes.** Your control loop runs fine at 100 Hz, then suddenly a frame takes 50ms instead of 10ms. You add logging, profile your code, check your algorithms - everything looks fine. The spike came from somewhere in the DDS layer, but good luck finding where.

**Unexplainable CPU usage.** Your node should be sleeping, waiting for data. Instead, `top` shows 15% CPU. DDS is doing... something. Discovery? Heartbeats? Quality of Service bookkeeping? I got frustrated and never got to know what really happened 🤷‍♂️

**QoS settings that don't behave as documented.** You set `reliability: reliable` expecting all messages to arrive. They will arrive but it will start pounding your cores. Then you got to `best_effort`, which in hindsight will not give its best effort.

**Configuration complexity.** To tune DDS performance, you need XML files with dozens of parameters: `max_samples`, `max_instances`, `resource_limits`, `history_depth`, transport settings. Is it too much to ask to just send bytes from one process to another? Why do I need to learn about the intricacies of DDS and how to tune them?

**Launch file spaghetti.** What started as "just Python" becomes unreadable nested substitutions, conditional includes, and opaque composition patterns. A simple "start these three nodes" becomes 200 lines. Well atleast the XML version still stayed. However, since the XML underneath calls the python API, the debugging has never been hard when a launch file crashes.

**Impossible debugging.** When something goes wrong, you face layers of abstraction: your code → rclcpp → rmw → DDS implementation → network stack. By the time you find the problem, your competitor has already gotten their project done and you have lost funding.

**The fundamental issue:** You spend time debugging middleware instead of building your robot.

## How Conduit is Different

| ROS 2 | Conduit |
|-------|---------|
| DDS with network abstraction | Shared memory only |
| Multiple QoS configurations | One simple model: latest N messages |
| XML/YAML DDS tuning files | No configuration needed |
| Python launch files | Simple YAML flow files |
| rmw → DDS → UDP/SHM | Direct shared memory access |
| Opaque latency sources | You see exactly where time goes |

### Shared Memory, No Network Stack

ROS 2's DDS abstraction was designed for distributed systems over networks. Even when communicating on the same machine, messages often go through serialization, the DDS layer, and potentially the network stack.

Conduit uses shared memory directly:

```
ROS 2 (same machine):
  Publisher → serialize → DDS → [maybe shared memory] → DDS → deserialize → Subscriber

Conduit:
  Publisher → write to shared memory → Subscriber reads directly
```

One write. Zero copies to read. No serialization unless you want it.

### No Configuration Needed

ROS 2 QoS matrix:

| Setting | Options |
|---------|---------|
| Reliability | RELIABLE, BEST_EFFORT |
| Durability | VOLATILE, TRANSIENT_LOCAL |
| History | KEEP_LAST(n), KEEP_ALL |
| Lifespan | Duration |
| Deadline | Duration |
| Liveliness | AUTOMATIC, MANUAL_BY_TOPIC |

Conduit configuration:

| Setting | Default |
|---------|---------|
| Slot count | 16 |
| Slot size | 4 KB |

That's it. Increase slot size for large messages. Increase slot count if you need more history.

### Readable Launch Files

**ROS 2 launch file:**
```python
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    config = PathJoinSubstitution([
        FindPackageShare('my_robot'),
        'config',
        'params.yaml'
    ])

    return LaunchDescription([
        DeclareLaunchArgument('use_sim', default_value='false'),
        Node(
            package='my_robot',
            executable='sensor_driver',
            parameters=[config],
            remappings=[('/input', '/sensors/raw')],
            condition=UnlessCondition(LaunchConfiguration('use_sim'))
        ),
        # ... 150 more lines
    ])
```

**Conduit flow file:**
```yaml
startup:
  - sensor_driver
  - wait: topic:sensors
  - perception
  - planning
  - control
```

### Predictable Latency

ROS 2 latency sources:
- DDS discovery protocol
- QoS negotiation
- Serialization/deserialization
- Possible network stack involvement
- DDS internal threading
- rmw layer overhead

Conduit latency sources:
- `memcpy` into shared memory
- `futex` wake system call

Conduit is purposely designed to be invisible. When something is slow, there will be tools provided to trace the issue.

### Lock-Free by Design

ROS 2 uses various locking mechanisms internally. A crashed or hung node can sometimes affect others through shared resources.

Conduit's ring buffer is lock-free:
- No mutexes
- No deadlocks possible
- Crashed publisher doesn't block subscribers
- Crashed subscriber doesn't affect publisher

### Zero CPU When Idle

ROS 2's DDS implementations often have background threads for discovery, liveliness, and other bookkeeping.

Conduit subscribers use Linux futex:
- Truly asleep in the kernel
- Zero CPU usage when waiting
- Instant wake when data arrives

## What Conduit Doesn't Do

Conduit is deliberately simple. It doesn't have:

- **Network transport** — Same-machine only (for now — see [Roadmap](roadmap.md))
- **Service calls** — Pub/sub only
- **Parameter server** — Use config files
- **Lifecycle management** — Nodes are just processes

If you need distributed systems across machines, ROS 2 is the right choice. If you need a simple, fast, debuggable IPC for a single robot, Conduit gets out of your way.

## When to Use Conduit

✅ **Good fit:**
- Single-machine robots
- Latency-critical control loops
- High-bandwidth sensors (cameras, LiDAR)
- Teams frustrated with DDS debugging
- Projects where simplicity matters

❌ **Not a fit:**
- Multi-machine distributed systems
- Need for ROS 2 ecosystem (Nav2, MoveIt, etc.)
- Require network transport today
