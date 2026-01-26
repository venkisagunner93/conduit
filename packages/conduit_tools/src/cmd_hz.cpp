#include "conduit_tools/commands.hpp"
#include <conduit_core/log.hpp>
#include <conduit_core/node.hpp>
#include <deque>
#include <mutex>

namespace conduit::tools {

class HzNode : public Node {
public:
    HzNode(const std::string& topic) : topic_(topic) {
        subscribe(topic, &HzNode::on_message);
        loop(1.0, &HzNode::print_rate);
    }

private:
    void on_message(const Message& msg) {
        (void)msg;
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        timestamps_.push_back(now);

        // Keep only last second
        while (!timestamps_.empty() && (now - timestamps_.front()) > std::chrono::seconds(1)) {
            timestamps_.pop_front();
        }
    }

    void print_rate() {
        std::lock_guard<std::mutex> lock(mutex_);
        double hz = static_cast<double>(timestamps_.size());
        fmt::print("{}: {:.1f} Hz\n", topic_, hz);
    }

    std::string topic_;
    std::deque<std::chrono::steady_clock::time_point> timestamps_;
    std::mutex mutex_;
};

int cmd_hz(int argc, char** argv) {
    if (argc < 2) {
        log::error("Usage: conduit hz <topic>");
        return 1;
    }

    std::string topic = argv[1];

    try {
        HzNode node(topic);
        node.run();
    } catch (const std::exception& e) {
        log::error("Error: {}", e.what());
        return 1;
    }

    return 0;
}

}  // namespace conduit::tools
