#include <conduit_core/log.hpp>
#include <conduit_core/node.hpp>
#include <string>

using namespace conduit;

class HelloSubscriber : public Node {
public:
    HelloSubscriber() {
        subscribe("hello", &HelloSubscriber::on_message);
    }

private:
    void on_message(const Message& msg) {
        std::string content(static_cast<const char*>(msg.data), msg.size);
        log::info("Received [seq={} size={}]: {}", msg.sequence, msg.size, content);
    }
};

int main() {
    HelloSubscriber node;
    node.run();
    return 0;
}
