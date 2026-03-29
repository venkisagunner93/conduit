

# File node.hpp

[**File List**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**node.hpp**](node_8hpp.md)

[Go to the documentation of this file](node_8hpp.md)


```C++
#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "conduit_core/publisher.hpp"
#include "conduit_core/subscriber.hpp"

namespace conduit {

class Node {
public:
    Node();
    virtual ~Node();

    // No copy, no move (prevent slicing)
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(Node&&) = delete;

    void run();

    void stop();

    bool running() const;

protected:
    template<typename T, typename Func>
    void subscribe(const std::string& topic, Func T::* callback);

    template<typename MsgT, typename T>
    void subscribe(const std::string& topic, void (T::* callback)(const TypedMessage<MsgT>&));

    void subscribe(const std::string& topic, std::function<void(const Message&)> callback);

    template<typename T, typename Func>
    void loop(double rate_hz, Func T::* callback);

    void loop(double rate_hz, std::function<void()> callback);

    template<typename T>
    Publisher<T> advertise(const std::string& topic, const PublisherOptions& options = {});

private:
    struct Subscription {
        std::string topic;
        std::function<void(const Message&)> callback;
        std::unique_ptr<internal::Subscriber> subscriber;
        std::thread thread;
    };

    struct Loop {
        double rate_hz;
        std::chrono::nanoseconds period;
        std::function<void()> callback;
        std::thread thread;
    };

    std::vector<std::unique_ptr<Subscription>> subscriptions_;
    std::vector<std::unique_ptr<Loop>> loops_;
    std::atomic<bool> running_{false};

    void spin_subscription(Subscription* sub);
    void spin_loop(Loop* lp);

    // Signal handling
    static std::atomic<Node*> active_node_;
    static void signal_handler(int sig);
    void install_signal_handlers();
    void uninstall_signal_handlers();
};

// Template implementations
template<typename T, typename Func>
void Node::subscribe(const std::string& topic, Func T::* callback) {
    subscribe(topic, [this, callback](const Message& msg) {
        (static_cast<T*>(this)->*callback)(msg);
    });
}

template<typename MsgT, typename T>
void Node::subscribe(const std::string& topic, void (T::* callback)(const TypedMessage<MsgT>&)) {
    subscribe(topic, [this, callback](const Message& msg) {
        MsgT data = [&]() {
            if constexpr (std::is_base_of_v<FixedMessageType, MsgT>) {
                MsgT d;
                std::memcpy(&d, msg.data, sizeof(MsgT));
                return d;
            } else {
                return MsgT::deserialize(
                    static_cast<const uint8_t*>(msg.data), msg.size);
            }
        }();
        TypedMessage<MsgT> typed{std::move(data), msg.sequence, msg.timestamp_ns};
        (static_cast<T*>(this)->*callback)(typed);
    });
}

template<typename T, typename Func>
void Node::loop(double rate_hz, Func T::* callback) {
    loop(rate_hz, [this, callback]() {
        (static_cast<T*>(this)->*callback)();
    });
}

template<typename T>
Publisher<T> Node::advertise(const std::string& topic, const PublisherOptions& options) {
    return Publisher<T>(topic, options);
}

}  // namespace conduit
```


