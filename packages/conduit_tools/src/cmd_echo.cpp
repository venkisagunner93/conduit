#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <conduit_core/node.hpp>
#include <string>

namespace conduit::tools {

class EchoNode : public Node {
public:
    EchoNode(const std::string& topic) {
        subscribe(topic, &EchoNode::on_message);
    }

private:
    void on_message(const Message& msg) {
        fmt::print("[seq={} ts={} size={}]\n", msg.sequence, msg.timestamp_ns, msg.size);
        print_hex(msg.data, msg.size);
        fmt::print("---\n");
    }

    static void print_hex(const void* data, size_t size) {
        const auto* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; i++) {
            if (i > 0 && i % 16 == 0) fmt::print("\n");
            fmt::print("{:02x} ", bytes[i]);
        }
        fmt::print("\n");
    }
};

int cmd_echo(int argc, char** argv) {
    if (argc < 2) {
        log::error("Usage: conduit echo <topic>");
        return 1;
    }

    std::string topic = argv[1];

    try {
        EchoNode node(topic);
        node.run();
    } catch (const std::exception& e) {
        log::error("Error: {}", e.what());
        return 1;
    }

    return 0;
}

}  // namespace conduit::tools
