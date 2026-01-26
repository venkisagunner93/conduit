#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace conduit::tools {

int cmd_topics(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const fs::path shm_dir = "/dev/shm";
    const std::string prefix = "conduit_";

    if (!fs::exists(shm_dir)) {
        fmt::print("No active topics.\n");
        return 0;
    }

    bool found = false;
    for (const auto& entry : fs::directory_iterator(shm_dir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.rfind(prefix, 0) == 0) {
                fmt::print("{}\n", filename.substr(prefix.length()));
                found = true;
            }
        }
    }

    if (!found) {
        fmt::print("No active topics.\n");
    }

    return 0;
}

}  // namespace conduit::tools
