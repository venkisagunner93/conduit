# Installation

Conduit development happens inside a Docker container to ensure consistent tooling.

## Prerequisites

- Docker
- Bash shell

## Setup

### 1. Clone the repository

```bash
git clone https://github.com/example/conduit.git
cd conduit
```

### 2. Source the helper script

```bash
source conduit.sh
```

This adds two commands to your shell:

| Command | Description |
|---------|-------------|
| `cbuild` | Build the Docker image |
| `crun` | Run the container with workspace mounted |

### 3. Build the Docker image

```bash
cbuild
```

### 4. Enter the container

```bash
crun
```

You're now inside the container at `/home/docker_user/workspace`.

## Building Conduit

Inside the container, use the `forge` build tool:

```bash
# Build all packages
forge build

# Build specific package
forge build conduit_core

# Run tests
forge test

# List packages
forge list

# Clean build artifacts
forge clean
```

## Verify Installation

After building, run the demo:

**Terminal 1:**
```bash
./install/bin/demo-publisher
```

**Terminal 2:**
```bash
./install/bin/demo-subscriber
```

**Terminal 3:**
```bash
./install/bin/conduit topics
./install/bin/conduit hz hello
```

## Environment Setup

To use the built binaries, source the setup script:

```bash
source install/setup.bash
```

This adds `install/bin` to your PATH and `install/lib` to your LD_LIBRARY_PATH.
