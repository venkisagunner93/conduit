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

    // Run node (blocks until SIGINT/SIGTERM or stop() called)
    void run();

    // Stop node (can be called from any thread or signal handler)
    void stop();

    // Check if running
    bool running() const;

protected:
    // Subscribe to topic with member function callback
    template<typename T, typename Func>
    void subscribe(const std::string& topic, Func T::* callback);

    // Subscribe to topic with lambda/function
    void subscribe(const std::string& topic, std::function<void(const Message&)> callback);

    // Loop at fixed rate with member function callback
    template<typename T, typename Func>
    void loop(double rate_hz, Func T::* callback);

    // Loop at fixed rate with lambda/function
    void loop(double rate_hz, std::function<void()> callback);

    // Create publisher
    Publisher advertise(const std::string& topic, const PublisherOptions& options = {});

private:
    struct Subscription {
        std::string topic;
        std::function<void(const Message&)> callback;
        std::unique_ptr<Subscriber> subscriber;
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

template<typename T, typename Func>
void Node::loop(double rate_hz, Func T::* callback) {
    loop(rate_hz, [this, callback]() {
        (static_cast<T*>(this)->*callback)();
    });
}

}  // namespace conduit
