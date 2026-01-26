#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <string>

using namespace conduit;

void print_usage() {
    fmt::print("Usage: conduit <command> [args]\n");
    fmt::print("\n");
    fmt::print("Commands:\n");
    fmt::print("  topics             List active topics\n");
    fmt::print("  info <topic>       Show topic details\n");
    fmt::print("  echo <topic>       Print messages (hex)\n");
    fmt::print("  hz <topic>         Measure publish rate\n");
    fmt::print("  record             Record topics to MCAP\n");
    fmt::print("  launch <file>      Launch nodes from YAML\n");
    fmt::print("\n");
    fmt::print("Examples:\n");
    fmt::print("  conduit topics\n");
    fmt::print("  conduit echo imu\n");
    fmt::print("  conduit record -o recording.mcap imu lidar\n");
    fmt::print("  conduit launch system.yaml\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string cmd = argv[1];

    // Shift args for subcommand
    int sub_argc = argc - 1;
    char** sub_argv = argv + 1;

    if (cmd == "topics")   return tools::cmd_topics(sub_argc, sub_argv);
    if (cmd == "info")   return tools::cmd_info(sub_argc, sub_argv);
    if (cmd == "echo")   return tools::cmd_echo(sub_argc, sub_argv);
    if (cmd == "hz")     return tools::cmd_hz(sub_argc, sub_argv);
    if (cmd == "record") return tools::cmd_record(sub_argc, sub_argv);
    if (cmd == "launch") return tools::cmd_launch(sub_argc, sub_argv);

    if (cmd == "-h" || cmd == "--help" || cmd == "help") {
        print_usage();
        return 0;
    }

    log::error("Unknown command: {}", cmd);
    print_usage();
    return 1;
}
