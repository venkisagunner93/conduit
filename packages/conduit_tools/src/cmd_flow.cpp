#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <conduit_flow/executor.hpp>
#include <conduit_flow/parser.hpp>

#include <dirent.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <vector>

namespace conduit::tools {

// Resolve the flows directory relative to the binary: <bin>/../share/conduit/flows
static std::string flows_dir() {
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) return {};
    buf[len] = '\0';

    // Strip binary name to get bin/
    std::string exe(buf);
    auto pos = exe.rfind('/');
    if (pos == std::string::npos) return {};

    // bin/../share/conduit/flows
    return exe.substr(0, pos) + "/../share/conduit/flows";
}

// Resolve a flow name to a file path.
// Tries in order: exact path, name.flow.yaml in flows dir, name in flows dir
static std::string resolve_flow(const std::string& name) {
    // If it contains a slash or ends with .yaml, treat as a path
    if (name.find('/') != std::string::npos || name.find(".yaml") != std::string::npos) {
        return name;
    }

    std::string dir = flows_dir();
    if (dir.empty()) return name;

    // Try name.flow.yaml
    std::string candidate = dir + "/" + name + ".flow.yaml";
    if (access(candidate.c_str(), R_OK) == 0) return candidate;

    // Try name.yaml
    candidate = dir + "/" + name + ".yaml";
    if (access(candidate.c_str(), R_OK) == 0) return candidate;

    return name;
}

static std::vector<std::string> list_flows() {
    std::vector<std::string> flows;
    std::string dir = flows_dir();
    if (dir.empty()) return flows;

    DIR* d = opendir(dir.c_str());
    if (!d) return flows;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name = entry->d_name;
        // Strip .flow.yaml suffix for display
        const std::string suffix = ".flow.yaml";
        if (name.size() > suffix.size() &&
            name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
            flows.push_back(name.substr(0, name.size() - suffix.size()));
        }
    }
    closedir(d);
    return flows;
}

int cmd_flow(int argc, char** argv) {
    if (argc < 2) {
        log::error("Usage: conduit flow <name>");
        auto flows = list_flows();
        if (!flows.empty()) {
            fmt::print("\nAvailable flows:\n");
            for (auto& f : flows) fmt::print("  {}\n", f);
        }
        return 1;
    }

    std::string arg = argv[1];

    if (arg == "-h" || arg == "--help") {
        fmt::print("Usage: conduit flow <name>\n");
        fmt::print("\n");
        fmt::print("Run a flow file by name or path.\n");
        fmt::print("\n");
        fmt::print("  conduit flow demo          # runs demo.flow.yaml from installed flows\n");
        fmt::print("  conduit flow ./my.flow.yaml # runs a specific file\n");
        fmt::print("\n");
        auto flows = list_flows();
        if (!flows.empty()) {
            fmt::print("Available flows:\n");
            for (auto& f : flows) fmt::print("  {}\n", f);
        }
        return 0;
    }

    if (arg == "--list") {
        auto flows = list_flows();
        for (auto& f : flows) fmt::print("{}\n", f);
        return 0;
    }

    // For bash completion
    if (arg == "--complete") {
        auto flows = list_flows();
        for (auto& f : flows) fmt::print("{}\n", f);
        return 0;
    }

    std::string file = resolve_flow(arg);

    try {
        log::info("Loading: {}", file);
        auto config = flow::parse_file(file);

        log::info("Startup steps: {}", config.startup.size());
        log::info("Shutdown steps: {}", config.shutdown.size());

        flow::Executor executor;
        return executor.run(config);

    } catch (const std::exception& e) {
        log::error("Error: {}", e.what());
        return 1;
    }
}

}  // namespace conduit::tools
