#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>

#include <yaml-cpp/yaml.h>

#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <cstring>

#include <algorithm>
#include <atomic>
#include <string>
#include <vector>

namespace conduit::tools {

struct NodeConfig {
    std::string name;
    std::string executable;
    std::vector<std::string> args;
};

struct Child {
    pid_t pid;
    std::string name;
};

static std::vector<Child> g_children;
static std::atomic<bool> g_stopping{false};

static void signal_handler(int) {
    if (g_stopping.exchange(true)) return;

    for (const auto& c : g_children) {
        kill(c.pid, SIGINT);
    }
}

static pid_t spawn(const NodeConfig& node) {
    pid_t pid = fork();

    if (pid < 0) {
        log::error("Fork failed for {}: {}", node.name, strerror(errno));
        return -1;
    }

    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(node.executable.c_str()));
        for (const auto& arg : node.args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        fprintf(stderr, "Exec failed for %s: %s\n", node.executable.c_str(), strerror(errno));
        _exit(127);
    }

    return pid;
}

int cmd_launch(int argc, char** argv) {
    if (argc < 2) {
        log::error("Usage: conduit launch <file.yaml>");
        return 1;
    }

    std::string file = argv[1];

    if (file == "-h" || file == "--help") {
        fmt::print("Usage: conduit launch <file.yaml>\n");
        fmt::print("\n");
        fmt::print("YAML format:\n");
        fmt::print("  nodes:\n");
        fmt::print("    - name: my_node\n");
        fmt::print("      executable: my_node_bin\n");
        fmt::print("      args: [\"--arg\", \"value\"]\n");
        return 0;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        YAML::Node config = YAML::LoadFile(file);

        if (!config["nodes"]) {
            log::error("Missing 'nodes' in launch file");
            return 1;
        }

        std::vector<NodeConfig> nodes;
        for (const auto& n : config["nodes"]) {
            NodeConfig nc;
            nc.name = n["name"].as<std::string>();
            nc.executable = n["executable"].as<std::string>();
            if (n["args"]) {
                for (const auto& a : n["args"]) {
                    nc.args.push_back(a.as<std::string>());
                }
            }
            nodes.push_back(nc);
        }

        log::info("Launching {} nodes...", nodes.size());

        for (const auto& node : nodes) {
            pid_t pid = spawn(node);
            if (pid > 0) {
                g_children.push_back({pid, node.name});
                log::info("Started {} (pid: {})", node.name, pid);
            }
        }

        if (g_children.empty()) {
            log::error("No nodes started");
            return 1;
        }

        log::info("Press Ctrl+C to stop.");

        while (!g_children.empty()) {
            int status;
            pid_t pid = waitpid(-1, &status, 0);

            if (pid < 0) {
                if (errno == EINTR) continue;
                break;
            }

            auto it = std::find_if(g_children.begin(), g_children.end(),
                [pid](const Child& c) { return c.pid == pid; });

            if (it != g_children.end()) {
                if (WIFEXITED(status)) {
                    log::info("{} exited (code: {})", it->name, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    log::info("{} killed (signal: {})", it->name, WTERMSIG(status));
                }
                g_children.erase(it);
            }

            if (!g_stopping && !g_children.empty()) {
                log::warn("Node exited unexpectedly, stopping all...");
                g_stopping = true;
                for (const auto& c : g_children) {
                    kill(c.pid, SIGINT);
                }
            }
        }

        log::info("All nodes stopped.");

    } catch (const YAML::Exception& e) {
        log::error("YAML error: {}", e.what());
        return 1;
    } catch (const std::exception& e) {
        log::error("Error: {}", e.what());
        return 1;
    }

    return 0;
}

}  // namespace conduit::tools
