# CLI Tools

Conduit provides a single `conduit` binary with subcommands for monitoring and recording.

## Installation

After building, the binary is at `install/bin/conduit`. Add to PATH:

```bash
source install/setup.bash
```

## Commands

| Command | Description |
|---------|-------------|
| `conduit topics` | List active topics |
| `conduit info <topic>` | Show topic metadata |
| `conduit echo <topic>` | Print messages (hex dump) |
| `conduit hz <topic>` | Measure publication rate |
| `conduit record` | Record topics to MCAP file |
| `conduit flow` | Run a flow file |

---

## conduit topics

List all active topics in `/dev/shm/conduit_*`.

```bash
$ conduit topics
hello
imu
camera
```

---

## conduit info

Show metadata for a topic.

```bash
$ conduit info hello
Topic: hello
  Slot count: 16
  Slot size: 4096
  Max message: 4076
  Subscribers: 1
  Messages: 42
```

---

## conduit echo

Print messages as they arrive (hex dump format).

```bash
$ conduit echo hello
[seq=0 ts=1234567890] size=12
  48 65 6c 6c 6f 20 57 6f 72 6c 64 21  Hello World!
[seq=1 ts=1234567891] size=12
  48 65 6c 6c 6f 20 57 6f 72 6c 64 21  Hello World!
^C
```

Press Ctrl+C to stop.

---

## conduit hz

Measure the publication rate of a topic.

```bash
$ conduit hz imu
Measuring imu...
  Rate: 100.2 Hz (avg over 50 messages)
  Rate: 99.8 Hz (avg over 100 messages)
  Rate: 100.0 Hz (avg over 150 messages)
^C
```

---

## conduit record

Record topics to an MCAP file.

```bash
# Record specific topics
conduit record -o data.mcap imu camera

# Record all topics
conduit record -o data.mcap --all
```

**Options:**

| Option | Description |
|--------|-------------|
| `-o, --output FILE` | Output MCAP file (required) |
| `--all` | Record all active topics |

Press Ctrl+C to stop recording.

**Output format:** [MCAP](https://mcap.dev) with Zstd compression.

---

## conduit flow

Run a flow file to start/stop multiple nodes.

```bash
conduit flow system.flow.yaml
```

See [Flow Files](flow.md) for the flow file format.
