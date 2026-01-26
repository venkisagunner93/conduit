# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Conduit** is a lightweight, zero-copy pub/sub middleware for robotics.

### Goals
- Zero-copy shared memory transport
- Lock-free SPMC (single producer, multiple consumer)
- Zero CPU when idle (futex-based signaling)
- Built-in recording and replay
- Simple node abstraction for users
- Backward-compatible message format (user chooses serialization)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         User Code                           │
│   class MyNode : public conduit::Node {                     │
│       subscribe("imu", &MyNode::on_imu);                    │
│   };                                                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      conduit::Node                          │
│   - Thread per subscription                                 │
│   - Callbacks dispatched automatically                      │
│   - run() for live, replay() for recorded data              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Publisher / Subscriber                     │
│   - Publisher: write + futex wake                           │
│   - Subscriber: futex wait + read                           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    SPMC Ring Buffer                         │
│   - Lock-free                                               │
│   - Cache-line aligned                                      │
│   - Sequence validation for overwrite detection             │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                /dev/shm/conduit/{topic}                     │
└─────────────────────────────────────────────────────────────┘
```

## Package Structure

| Package | Description |
|---------|-------------|
| conduit_core | Ring buffer, shared memory, futex, pub/sub, node, logging |
| conduit_tank | MCAP-based recording with Zstd/LZ4 compression |
| conduit_demo | Example publisher and subscriber applications |
| conduit_tools | CLI tools (list, info, echo, hz, record, launch) |

## Development Phases

| Phase | Deliverable | Status |
|-------|-------------|--------|
| 1 | Lock-free SPMC ring buffer | Complete |
| 2 | Shared memory region | Complete |
| 3 | Futex signaling | Complete |
| 4 | Publisher / Subscriber | Complete |
| 5 | Message framing (timestamp, sequence) | Complete |
| 6 | Node abstraction (thread per subscription, callbacks) | Complete |
| 7 | Recording / Replay | Complete |
| 8 | CLI tools (list, info, echo, hz, record, launch) | Complete |

## CLI Tools

Single `conduit` binary with subcommands:

```bash
conduit topics                       # List active topics
conduit info <topic>                 # Show topic metadata
conduit echo <topic>                 # Echo messages with hex dump
conduit hz <topic>                   # Measure publication rate (Hz)
conduit record -o <file> <topics...> # Record topics to MCAP file
conduit launch <config.yaml>         # Launch multiple nodes from YAML
```

## Ring Buffer Configuration

- **Slot count**: Configurable power of 2, default 16
- **Max message size**: Configurable, default 4096 bytes
- **Slot header**: 20 bytes (size: 4B, sequence: 8B, timestamp_ns: 8B)
- **Cache-line aligned**: 64 bytes for performance
- **Max subscribers**: 16 concurrent readers

## Message Format

Messages include automatic framing:
- `timestamp_ns`: uint64_t nanoseconds (CLOCK_MONOTONIC_RAW)
- `sequence`: uint64_t incremental per message
- `size`: uint32_t payload size
- `data`: User-defined payload (user chooses serialization)

## Development Environment

### Docker Commands (from host, after sourcing conduit.sh)
```bash
source conduit.sh
cbuild    # Build Docker image from docker/Dockerfile.conduit
crun      # Run container with workspace mounted at /home/docker_user/workspace
```

### Build System (inside container)
```bash
forge build          # Build all packages
forge build <pkg>    # Build specific package
forge test           # Run all tests
forge list           # List packages and build order
forge clean          # Clean build artifacts
```

### Debugging Tools
- **GDB**: `gdb ./binary`
- **Valgrind**: `valgrind ./binary`

## Toolchain

- **Build**: CMake 3.16+ with Ninja backend
- **Compiler**: GCC/G++ with C++17 standard
- **Testing**: Google Test (gtest) and Google Mock (gmock)
- **Debugging**: GDB, Valgrind

## Dependencies

- **fmt**: Formatting library (v10.2.1, fetched via CMake)
- **MCAP**: Recording format (v1.4.0, fetched via CMake)
- **lz4**: Compression (system package)
- **zstd**: Compression (system package)
- **yaml-cpp**: YAML parsing for launch tool (system package)

## Tests

```bash
cd build && ctest --output-on-failure
```

Test executables:
- `ring_buffer_test` - Lock-free ring buffer operations
- `shm_region_test` - Shared memory management
- `futex_test` - Futex signaling mechanisms
- `pubsub_test` - Publisher/Subscriber integration
- `node_test` - Node threading and callbacks
- `tank_test` - MCAP recording

## Runtime

- All development happens inside Docker container
- Workspace is volume-mounted, changes persist on host
- Container runs as non-root user `docker_user` with sudo access
- Shared memory via `/dev/shm/conduit_{topic}`

## Quick Start (Demo)

Terminal 1 - Start publisher:
```bash
./install/bin/demo-publisher
```

Terminal 2 - Start subscriber:
```bash
./install/bin/demo-subscriber
```

Terminal 3 - Monitor with CLI tools:
```bash
./install/bin/conduit topics
./install/bin/conduit hz hello
./install/bin/conduit echo hello
```
