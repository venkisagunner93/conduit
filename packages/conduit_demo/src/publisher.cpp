#include <conduit_core/log.hpp>
#include <conduit_core/node.hpp>
#include <optional>

using namespace conduit;

class HelloPublisher : public Node {
public:
    HelloPublisher() {
        pub_.emplace(advertise("hello"));
        loop(1.0, &HelloPublisher::publish);
    }

private:
    void publish() {
        std::string msg = fmt::format("Hello World #{}", count_++);
        pub_->publish(msg.data(), msg.size());
        log::info("Published: {}", msg);
    }

    std::optional<Publisher> pub_;
    uint64_t count_ = 0;
};

int main() {
    HelloPublisher node;
    node.run();
    return 0;
}
