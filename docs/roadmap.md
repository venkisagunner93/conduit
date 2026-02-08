# Roadmap

Planned features for Conduit.

## conduit_joint

**Status:** Planned

Joint is Conduit's bridge to the outside world.

### The Problem

Conduit uses shared memory — fast, but limited to one machine. Real robots need:

- Telemetry to a base station
- Commands from a remote operator
- Data to cloud services
- Integration with other systems

### The Solution

Joint provides network transports that bridge Conduit topics:

```
┌─────────────────────────────────────────────────────────────┐
│                        Robot                                │
│                                                             │
│   [Sensors] → conduit → [Joint Bridge] ──────────────────┐  │
│                              ↑                           │  │
│   [Actuators] ← conduit ← [Joint Bridge] ←────────────┐  │  │
│                                                       │  │  │
└───────────────────────────────────────────────────────┼──┼──┘
                                                        │  │
                                            Network     │  │
                                                        │  │
┌───────────────────────────────────────────────────────┼──┼──┐
│                    Base Station                       │  │  │
│                                                       ▼  │  │
│   [Visualization] ← [Joint Client] ←──────────────────┘  │  │
│                                                          │  │
│   [Operator UI] → [Joint Client] ────────────────────────┘  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Planned Transports

| Transport | Use Case |
|-----------|----------|
| TCP | Reliable data over local network |
| WebSocket | Browser-based visualization |
| UDP | Low-latency telemetry (lossy OK) |
| ZeroMQ | High-performance pub/sub |

### Configuration

```yaml
# joint.yaml
bridges:
  - topic: camera
    transport: tcp
    port: 9000
    compress: true

  - topic: telemetry
    transport: websocket
    port: 8080

  - topic: cmd_vel
    transport: udp
    port: 9001
    direction: inbound
```

### Use Cases

**Remote visualization:**
```bash
# On robot
joint bridge --config joint.yaml

# On laptop
joint subscribe tcp://robot:9000/camera | foxglove
```

**Teleop over network:**
```bash
# Base station sends commands
joint publish tcp://robot:9001/cmd_vel
```

**Cloud telemetry:**
```bash
# Stream to cloud service
joint bridge --topic telemetry --to wss://cloud.example.com/ingest
```

## Future Ideas

These are under consideration but not yet planned:

| Feature | Description |
|---------|-------------|
| **Python bindings** | `pip install conduit` with Pythonic API |
| **Message playback** | Replay MCAP files as live topics |
| **Time sync** | Coordinated timestamps across machines |
| **Priority topics** | Real-time scheduling for critical data |
| **Encryption** | Secure network transport |
| **Message introspection** | CLI tools that understand typed messages |

## Contributing

Want to help? Areas where contributions are welcome:

- Testing on different Linux distributions
- Performance benchmarking
- Documentation improvements
- Bug reports and feature requests

See the [GitHub repository](https://github.com/venkisagunner93/conduit) for issues and discussions.
