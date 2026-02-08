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

/// @brief Base class for conduit processing nodes.
///
/// A Node manages subscriptions and publish loops. Each subscription
/// runs on its own thread, with callbacks dispatched automatically
/// when messages arrive. Call run() to start all threads and block
/// until SIGINT/SIGTERM or stop() is called.
///
/// @code
/// class MyNode : public conduit::Node {
/// public:
///     MyNode() {
///         subscribe<Imu>("imu", &MyNode::on_imu);
///         pub_ = advertise<Twist>("cmd_vel");
///         loop(10.0, &MyNode::tick);
///     }
/// private:
///     void on_imu(const TypedMessage<Imu>& msg) { /* ... */ }
///     void tick() { pub_.publish(Twist{}); }
///     Publisher<Twist> pub_;
/// };
/// @endcode
///
/// @see Publisher, Subscriber
class Node {
public:
    /// @brief Construct a node.
    Node();
    virtual ~Node();

    // No copy, no move (prevent slicing)
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(Node&&) = delete;

    /// @brief Run the node, blocking until SIGINT/SIGTERM or stop() is called.
    ///
    /// Starts all subscription threads and loop threads, installs signal
    /// handlers, and blocks the calling thread.
    void run();

    /// @brief Stop the node (can be called from any thread or signal handler).
    void stop();

    /// @brief Check if the node is currently running.
    /// @return true if run() has been called and stop() has not yet completed.
    bool running() const;

protected:
    /// @brief Subscribe to a topic with a member function callback (raw).
    /// @tparam T Derived Node type.
    /// @tparam Func Member function pointer type.
    /// @param topic Topic name to subscribe to.
    /// @param callback Member function to call on each message.
    template<typename T, typename Func>
    void subscribe(const std::string& topic, Func T::* callback);

    /// @brief Subscribe to a topic with a typed member function callback.
    ///
    /// Messages are automatically deserialized to MsgT before invoking
    /// the callback.
    ///
    /// @tparam MsgT Message type to deserialize into.
    /// @tparam T Derived Node type.
    /// @param topic Topic name to subscribe to.
    /// @param callback Member function receiving TypedMessage<MsgT>.
    template<typename MsgT, typename T>
    void subscribe(const std::string& topic, void (T::* callback)(const TypedMessage<MsgT>&));

    /// @brief Subscribe to a topic with a lambda or std::function callback (raw).
    /// @param topic Topic name to subscribe to.
    /// @param callback Function invoked with each raw Message.
    void subscribe(const std::string& topic, std::function<void(const Message&)> callback);

    /// @brief Register a fixed-rate loop with a member function callback.
    /// @tparam T Derived Node type.
    /// @tparam Func Member function pointer type.
    /// @param rate_hz Loop frequency in Hz.
    /// @param callback Member function to call each iteration.
    template<typename T, typename Func>
    void loop(double rate_hz, Func T::* callback);

    /// @brief Register a fixed-rate loop with a lambda or std::function callback.
    /// @param rate_hz Loop frequency in Hz.
    /// @param callback Function to call each iteration.
    void loop(double rate_hz, std::function<void()> callback);

    /// @brief Create a typed publisher for the given topic.
    /// @tparam T Message type to publish.
    /// @param topic Topic name.
    /// @param options Publisher configuration.
    /// @return A Publisher<T> ready to publish messages.
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
