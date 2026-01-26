#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <conduit_tank/tank.hpp>
#include <csignal>
#include <string>
#include <thread>
#include <vector>

namespace conduit::tools {

static Tank* g_tank = nullptr;

static void signal_handler(int) {
    if (g_tank) g_tank->stop();
}

int cmd_record(int argc, char** argv) {
    std::string output;
    std::vector<std::string> topics;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            fmt::print("Usage: conduit record -o <output.mcap> <topic1> [topic2] ...\n");
            return 0;
        } else if (arg[0] != '-') {
            topics.push_back(arg);
        } else {
            log::error("Unknown option: {}", arg);
            return 1;
        }
    }

    if (output.empty()) {
        log::error("Output file required (-o)");
        return 1;
    }

    if (topics.empty()) {
        log::error("At least one topic required");
        return 1;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        Tank tank(output);
        g_tank = &tank;

        for (const auto& topic : topics) {
            tank.add_topic(topic);
            log::info("Recording topic: {}", topic);
        }

        tank.start();
        log::info("Recording to: {}", output);
        log::info("Press Ctrl+C to stop.");

        while (tank.recording()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            log::info("Messages: {}", tank.message_count());
        }

        log::info("Stopped. Total: {}", tank.message_count());

    } catch (const std::exception& e) {
        log::error("Error: {}", e.what());
        return 1;
    }

    return 0;
}

}  // namespace conduit::tools
