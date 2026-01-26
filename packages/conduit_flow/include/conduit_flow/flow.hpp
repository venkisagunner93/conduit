#pragma once

#include <chrono>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace conduit::flow {

// Node configuration
struct NodeConfig {
    std::string name;
    std::string exec;
    std::vector<std::string> args;
    std::map<std::string, std::string> env;
    std::string working_dir;
};

// Wait for duration
struct WaitDuration {
    std::chrono::milliseconds duration;
};

// Wait for topics to exist
struct WaitTopics {
    std::vector<std::string> topics;
    std::chrono::milliseconds timeout{30000};
};

// Parallel group
struct Group {
    std::vector<NodeConfig> nodes;
};

// A step in the flow
using Step = std::variant<NodeConfig, WaitDuration, WaitTopics, Group>;

// Complete flow configuration
struct FlowConfig {
    std::vector<Step> startup;
    std::vector<Step> shutdown;
};

}  // namespace conduit::flow
