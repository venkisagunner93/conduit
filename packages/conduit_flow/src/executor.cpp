#include "conduit_flow/executor.hpp"
#include <conduit_core/exceptions.hpp>
#include <conduit_core/internal/shm_region.hpp>
#include <conduit_core/log.hpp>

#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <csignal>
#include <cstring>
#include <thread>

namespace conduit::flow {

struct ProcessInfo {
    pid_t pid;
    std::string name;
};

struct Executor::Impl {
    ExecutorOptions options;
    std::vector<ProcessInfo> processes;
    std::atomic<bool> running{false};
    std::atomic<bool> shutdown_requested{false};

    static Impl* g_instance;

    pid_t spawn(const NodeConfig& node);
    void stop_process(const ProcessInfo& proc);
    bool wait_for_topic(const std::string& topic, std::chrono::milliseconds timeout);
    void execute_step(const Step& step);
    void execute_shutdown_step(const Step& step);

    static void signal_handler(int sig);
};

Executor::Impl* Executor::Impl::g_instance = nullptr;

void Executor::Impl::signal_handler(int sig) {
    (void)sig;
    if (g_instance) {
        g_instance->shutdown_requested = true;
    }
}

pid_t Executor::Impl::spawn(const NodeConfig& node) {
    pid_t pid = fork();

    if (pid < 0) {
        throw ConduitError("Fork failed: " + std::string(strerror(errno)));
    }

    if (pid == 0) {
        for (const auto& [key, value] : node.env) {
            setenv(key.c_str(), value.c_str(), 1);
        }

        if (!node.working_dir.empty()) {
            if (chdir(node.working_dir.c_str()) < 0) {
                fprintf(stderr, "chdir failed: %s\n", strerror(errno));
                _exit(127);
            }
        }

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(node.exec.c_str()));
        for (const auto& arg : node.args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());

        fprintf(stderr, "Exec failed for %s: %s\n", node.exec.c_str(), strerror(errno));
        _exit(127);
    }

    return pid;
}

void Executor::Impl::stop_process(const ProcessInfo& proc) {
    kill(proc.pid, SIGINT);

    auto start = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds(5);

    while (true) {
        int status;
        pid_t result = waitpid(proc.pid, &status, WNOHANG);

        if (result == proc.pid) {
            if (WIFEXITED(status)) {
                log::info("{} exited (code: {})", proc.name, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                log::info("{} killed (signal: {})", proc.name, WTERMSIG(status));
            }
            return;
        }

        if (std::chrono::steady_clock::now() - start > timeout) {
            log::warn("{} did not stop, sending SIGKILL", proc.name);
            kill(proc.pid, SIGKILL);
            waitpid(proc.pid, &status, 0);
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

bool Executor::Impl::wait_for_topic(const std::string& topic,
                                    std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();

    while (true) {
        if (shutdown_requested) {
            return false;
        }

        if (internal::ShmRegion::exists(topic)) {
            return true;
        }

        if (std::chrono::steady_clock::now() - start > timeout) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Executor::Impl::execute_step(const Step& step) {
    if (shutdown_requested) return;

    std::visit(
        [this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, NodeConfig>) {
                log::info("Starting: {}", arg.name);
                pid_t pid = spawn(arg);
                processes.push_back({pid, arg.name});
                log::info("Started {} (pid: {})", arg.name, pid);

            } else if constexpr (std::is_same_v<T, WaitDuration>) {
                log::info("Waiting: {} ms", arg.duration.count());
                auto end = std::chrono::steady_clock::now() + arg.duration;
                while (std::chrono::steady_clock::now() < end && !shutdown_requested) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }

            } else if constexpr (std::is_same_v<T, WaitTopics>) {
                log::info("Waiting for topics...");
                for (const auto& topic : arg.topics) {
                    log::info("  waiting: {}", topic);
                    if (!wait_for_topic(topic, arg.timeout)) {
                        if (!shutdown_requested) {
                            throw ConduitError("Timeout waiting for topic: " + topic);
                        }
                        return;
                    }
                    log::info("  ready: {}", topic);
                }

            } else if constexpr (std::is_same_v<T, Group>) {
                std::vector<std::string> names;
                for (const auto& node : arg.nodes) {
                    names.push_back(node.name);
                }
                log::info("Starting group: {}", fmt::join(names, ", "));

                for (const auto& node : arg.nodes) {
                    pid_t pid = spawn(node);
                    processes.push_back({pid, node.name});
                    log::info("Started {} (pid: {})", node.name, pid);
                }
            }
        },
        step);
}

void Executor::Impl::execute_shutdown_step(const Step& step) {
    std::visit(
        [this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, NodeConfig>) {
                auto it = std::find_if(processes.begin(), processes.end(),
                                       [&](const ProcessInfo& p) { return p.name == arg.name; });

                if (it != processes.end()) {
                    log::info("Stopping: {}", it->name);
                    stop_process(*it);
                    processes.erase(it);
                }

            } else if constexpr (std::is_same_v<T, WaitDuration>) {
                log::info("Waiting: {} ms", arg.duration.count());
                std::this_thread::sleep_for(arg.duration);

            } else if constexpr (std::is_same_v<T, Group>) {
                std::vector<std::string> names;
                for (const auto& node : arg.nodes) {
                    names.push_back(node.name);
                }
                log::info("Stopping group: {}", fmt::join(names, ", "));

                for (const auto& node : arg.nodes) {
                    auto it =
                        std::find_if(processes.begin(), processes.end(),
                                     [&](const ProcessInfo& p) { return p.name == node.name; });

                    if (it != processes.end()) {
                        stop_process(*it);
                        processes.erase(it);
                    }
                }
            }
        },
        step);
}

Executor::Executor(ExecutorOptions options) : impl_(std::make_unique<Impl>()) {
    impl_->options = options;
}

Executor::~Executor() {
    shutdown();
}

int Executor::run(const FlowConfig& config) {
    impl_->running = true;
    impl_->shutdown_requested = false;

    Impl::g_instance = impl_.get();
    std::signal(SIGINT, Impl::signal_handler);
    std::signal(SIGTERM, Impl::signal_handler);

    try {
        log::info("=== Startup ===");
        for (const auto& step : config.startup) {
            impl_->execute_step(step);
            if (impl_->shutdown_requested) break;
        }

        if (!impl_->shutdown_requested) {
            log::info("=== All nodes running ===");
        }

        while (!impl_->shutdown_requested) {
            int status;
            pid_t pid = waitpid(-1, &status, WNOHANG);

            if (pid > 0) {
                auto it =
                    std::find_if(impl_->processes.begin(), impl_->processes.end(),
                                 [pid](const ProcessInfo& p) { return p.pid == pid; });

                if (it != impl_->processes.end()) {
                    if (WIFEXITED(status)) {
                        log::error("{} exited unexpectedly (code: {})", it->name,
                                   WEXITSTATUS(status));
                    } else if (WIFSIGNALED(status)) {
                        log::error("{} killed (signal: {})", it->name, WTERMSIG(status));
                    }
                    impl_->processes.erase(it);

                    log::warn("Node crashed, initiating shutdown...");
                    impl_->shutdown_requested = true;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        log::info("=== Shutdown ===");
        for (const auto& step : config.shutdown) {
            impl_->execute_shutdown_step(step);
        }

        for (const auto& proc : impl_->processes) {
            log::warn("Force stopping: {}", proc.name);
            impl_->stop_process(proc);
        }
        impl_->processes.clear();

        log::info("=== Shutdown complete ===");

    } catch (const std::exception& e) {
        log::error("Error: {}", e.what());

        for (const auto& proc : impl_->processes) {
            kill(proc.pid, SIGKILL);
        }

        impl_->running = false;
        Impl::g_instance = nullptr;
        return 1;
    }

    impl_->running = false;
    Impl::g_instance = nullptr;
    return 0;
}

void Executor::shutdown() {
    impl_->shutdown_requested = true;
}

bool Executor::running() const {
    return impl_->running;
}

}  // namespace conduit::flow
