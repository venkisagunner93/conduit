#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <conduit_flow/executor.hpp>
#include <conduit_flow/parser.hpp>
#include <string>

namespace conduit::tools {

int cmd_flow(int argc, char** argv) {
    if (argc < 2) {
        log::error("Usage: conduit flow <file.flow.yaml>");
        return 1;
    }

    std::string file = argv[1];

    if (file == "-h" || file == "--help") {
        fmt::print("Usage: conduit flow <file.flow.yaml>\n");
        fmt::print("\n");
        fmt::print("Run a flow file.\n");
        fmt::print("\n");
        fmt::print("Flow file format:\n");
        fmt::print("  startup:\n");
        fmt::print("    - node_name\n");
        fmt::print("    - name: my_node\n");
        fmt::print("      exec: my_executable\n");
        fmt::print("      args: [\"--flag\", \"value\"]\n");
        fmt::print("    - wait: 1s\n");
        fmt::print("    - wait: topic:my_topic\n");
        fmt::print("    - group:\n");
        fmt::print("        - node_a\n");
        fmt::print("        - node_b\n");
        fmt::print("\n");
        fmt::print("  shutdown:  # optional, default is reverse of startup\n");
        fmt::print("    - node_name\n");
        return 0;
    }

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
