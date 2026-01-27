# Flow Files

Flow files define startup and shutdown sequences for multi-node systems.

## Overview

A flow file is a YAML file that specifies:

- **startup** - Steps to run when starting
- **shutdown** - Steps to run when stopping (optional)

## Basic Example

```yaml
# system.flow.yaml
startup:
  - sensor_driver
  - wait: topic:imu
  - perception
  - planning
  - control
```

Run with:

```bash
conduit flow system.flow.yaml
```

## Step Types

### Node

Start a process.

**Simple form:**
```yaml
startup:
  - my_node  # Runs executable named "my_node"
```

**Full form:**
```yaml
startup:
  - name: my_node
    exec: /path/to/executable
    args: ["--flag", "value"]
    env:
      MY_VAR: "value"
    working_dir: /path/to/dir
```

| Field | Description |
|-------|-------------|
| `name` | Node identifier (for logging) |
| `exec` | Executable path (default: same as name) |
| `args` | Command-line arguments |
| `env` | Environment variables |
| `working_dir` | Working directory |

### Wait Duration

Pause for a specified time.

```yaml
startup:
  - sensor_driver
  - wait: 1s           # Wait 1 second
  - wait: 500ms        # Wait 500 milliseconds
  - perception
```

Supported units: `s`, `ms`, `us`, `ns`

### Wait for Topic

Wait until a topic exists.

```yaml
startup:
  - camera_driver
  - wait: topic:camera    # Wait for "camera" topic
  - image_processor
```

Wait for multiple topics:

```yaml
startup:
  - wait: [topic:imu, topic:camera]  # Wait for both
```

### Group

Start multiple nodes simultaneously.

```yaml
startup:
  - group:
      - left_camera
      - right_camera
  - stereo_processor
```

Nodes in a group start in parallel. The flow continues after all have started.

## Shutdown Sequence

By default, shutdown is the reverse of startup (nodes stopped in reverse order).

Specify a custom shutdown:

```yaml
startup:
  - database
  - web_server
  - workers

shutdown:
  - workers
  - wait: 1s
  - web_server
  - wait: 500ms
  - database
```

## Complete Example

```yaml
# robot.flow.yaml
startup:
  # Start hardware drivers
  - group:
      - imu_driver
      - lidar_driver
      - camera_driver

  # Wait for sensor data
  - wait: [topic:imu, topic:lidar, topic:camera]

  # Start perception
  - localization
  - wait: topic:pose
  - obstacle_detection

  # Start planning and control
  - path_planner
  - wait: topic:path
  - controller

shutdown:
  # Graceful shutdown: control first, then perception, then drivers
  - controller
  - path_planner
  - wait: 100ms
  - obstacle_detection
  - localization
  - wait: 100ms
  - group:
      - camera_driver
      - lidar_driver
      - imu_driver
```

## Signal Handling

When you press Ctrl+C:

1. Flow executor receives SIGINT
2. Runs shutdown sequence
3. Sends SIGTERM to remaining nodes
4. Waits for nodes to exit
5. Exits cleanly

## Error Handling

If a node fails to start:

- Error is logged
- Remaining startup steps are skipped
- Shutdown sequence runs
- Exit with non-zero status

## Debugging

See which nodes are running:

```bash
ps aux | grep conduit
```

Check topic activity:

```bash
conduit topics
conduit hz <topic>
```
