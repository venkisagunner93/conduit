# CLI Tools

The `conduit` command provides tools for monitoring and recording.

```bash
conduit <command> [options]
```

## topics

List all active topics.

```bash
$ conduit topics
camera
imu
lidar
odom
```

Topics are discovered by scanning `/dev/shm/conduit_*`.

## info

Show metadata for a topic.

```bash
$ conduit info imu
Topic: imu
  Slot count: 16
  Slot size: 4096
  Max message: 4076 bytes
  Subscribers: 2
  Messages published: 15420
```

## echo

Print messages as they arrive.

```bash
$ conduit echo imu
[seq=1000 ts=1705312345678901234] size=48
  00 00 00 00 00 00 f0 3f  00 00 00 00 00 00 00 00  ........?.......
  9a 99 99 3e cd cc 4c 3f  00 00 80 3f 00 00 00 00  ...>..L?...?....
  00 00 00 00 00 00 00 00                           ........
[seq=1001 ts=1705312345688901234] size=48
  ...
^C
```

Press `Ctrl+C` to stop.

## hz

Measure publication rate.

```bash
$ conduit hz imu
Measuring imu...
  Rate: 99.8 Hz (avg 50 msgs)
  Rate: 100.1 Hz (avg 100 msgs)
  Rate: 100.0 Hz (avg 150 msgs)
^C
```

Useful for verifying sensors and publishers are running at expected rates.

## record

Record topics to an MCAP file.

```bash
# Record specific topics
conduit record -o data.mcap imu camera odom

# Record all active topics
conduit record -o data.mcap --all
```

Press `Ctrl+C` to stop recording.

**Output format:** [MCAP](https://mcap.dev) with Zstd compression.

**Options:**

| Option | Description |
|--------|-------------|
| `-o, --output FILE` | Output file path (required) |
| `--all` | Record all active topics |

## flow

Run a flow file to orchestrate multiple nodes.

```bash
conduit flow robot.flow.yaml
```

Press `Ctrl+C` for graceful shutdown (runs shutdown sequence).

### Flow File Format

```yaml
startup:
  - driver_node                    # Simple: exec = "driver_node"
  - name: perception               # With options
    exec: /path/to/perception
    args: ["--config", "params.yaml"]
    env:
      MODEL_PATH: /models
  - wait: topic:camera             # Wait for topic to exist
  - wait: 500ms                    # Wait duration
  - wait: [topic:a, topic:b]       # Wait for multiple topics
  - group:                         # Start in parallel
      - planner
      - controller

shutdown:                          # Optional (default: reverse of startup)
  - controller
  - wait: 100ms
  - planner
```

### Step Types

| Type | Example | Description |
|------|---------|-------------|
| Node (simple) | `- my_node` | Start executable `my_node` |
| Node (full) | `- name: x, exec: /path` | Start with options |
| Wait duration | `- wait: 1s` | Pause (s, ms, us, ns) |
| Wait topic | `- wait: topic:imu` | Wait until topic exists |
| Wait multiple | `- wait: [topic:a, topic:b]` | Wait for all topics |
| Group | `- group: [a, b, c]` | Start nodes in parallel |

### Node Options

```yaml
- name: my_node           # Identifier for logging
  exec: /path/to/binary   # Executable (default: name)
  args: ["--flag", "val"] # Command line arguments
  env:                    # Environment variables
    KEY: value
  working_dir: /path      # Working directory
```

## Examples

### Monitor a Robot

```bash
# Terminal 1: Watch what topics exist
watch -n 1 conduit topics

# Terminal 2: Check IMU rate
conduit hz imu

# Terminal 3: See raw messages
conduit echo cmd_vel
```

### Record a Test Run

```bash
# Start recording
conduit record -o test_run_001.mcap imu camera odom cmd_vel

# ... run your test ...

# Ctrl+C to stop
# File saved: test_run_001.mcap
```

### Launch a System

```yaml
# robot.flow.yaml
startup:
  - imu_driver
  - camera_driver
  - wait: [topic:imu, topic:camera]
  - localization
  - wait: topic:pose
  - navigation
  - teleop
```

```bash
conduit flow robot.flow.yaml
# Ctrl+C for graceful shutdown
```
