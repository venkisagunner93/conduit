#pragma once

#include "conduit_flow/flow.hpp"
#include <memory>

namespace conduit::flow {

// Execution options
struct ExecutorOptions {
    bool verbose = true;
};

// Executor class
class Executor {
public:
    explicit Executor(ExecutorOptions options = {});
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    // Run the flow (blocks until shutdown)
    int run(const FlowConfig& config);

    // Request shutdown
    void shutdown();

    // Check if running
    bool running() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace conduit::flow
