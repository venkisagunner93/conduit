#include <conduit_core/log.hpp>
#include <conduit_core/node.hpp>
#include <conduit_types/primitives/uint.hpp>
#include <demo/string_msg.hpp>

using namespace conduit;

class HelloSubscriber : public Node {
public:
    HelloSubscriber() {
        subscribe<StringMsg>("hello", &HelloSubscriber::on_hello);
        subscribe<Uint>("counter", &HelloSubscriber::on_counter);
    }

private:
    void on_hello(const TypedMessage<StringMsg>& msg) {
        log::info("Received [seq={}]: {}", msg.sequence, msg.data.text);
    }

    void on_counter(const TypedMessage<Uint>& msg) {
        log::info("Counter: {}", msg.data.value);
    }
};

int main() {
    HelloSubscriber node;
    node.run();
    return 0;
}
