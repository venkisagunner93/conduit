#pragma once

#include "conduit_flow/flow.hpp"
#include <memory>

namespace conduit::flow {

/// @brief Options for the flow executor.
struct ExecutorOptions {
    bool verbose = true;  ///< Print startup/shutdown progress to stdout.
};

/// @brief Executes a flow configuration (startup and shutdown sequences).
///
/// Launches nodes as child processes, handles wait steps and parallel groups,
/// and manages graceful shutdown on SIGINT/SIGTERM.
class Executor {
public:
    /// @brief Construct an executor with the given options.
    /// @param options Execution options.
    explicit Executor(ExecutorOptions options = {});
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;

    /// @brief Run the flow (blocks until shutdown completes).
    /// @param config The flow configuration to execute.
    /// @return Exit code (0 on clean shutdown).
    int run(const FlowConfig& config);

    /// @brief Request graceful shutdown of all running nodes.
    void shutdown();

    /// @brief Check if the executor is currently running a flow.
    /// @return true if run() is active.
    bool running() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace conduit::flow
