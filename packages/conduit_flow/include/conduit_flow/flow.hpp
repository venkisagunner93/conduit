#pragma once

#include <chrono>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace conduit::flow {

/// @brief Configuration for a single node in a flow.
struct NodeConfig {
    std::string name;                            ///< Logical node name.
    std::string exec;                            ///< Executable path or command.
    std::vector<std::string> args;               ///< Command-line arguments.
    std::map<std::string, std::string> env;      ///< Environment variable overrides.
    std::string working_dir;                     ///< Working directory for the process.
};

/// @brief Wait step: pause for a fixed duration.
struct WaitDuration {
    std::chrono::milliseconds duration;  ///< Time to wait.
};

/// @brief Wait step: block until specified topics exist in shared memory.
struct WaitTopics {
    std::vector<std::string> topics;             ///< Topic names to wait for.
    std::chrono::milliseconds timeout{30000};    ///< Maximum wait time (default 30s).
};

/// @brief Parallel group: start multiple nodes simultaneously.
struct Group {
    std::vector<NodeConfig> nodes;  ///< Nodes to launch in parallel.
};

/// @brief A single step in a flow sequence.
using Step = std::variant<NodeConfig, WaitDuration, WaitTopics, Group>;

/// @brief Complete flow configuration with startup and shutdown sequences.
struct FlowConfig {
    std::vector<Step> startup;   ///< Ordered startup steps.
    std::vector<Step> shutdown;  ///< Ordered shutdown steps (default: reverse of startup).
};

}  // namespace conduit::flow
