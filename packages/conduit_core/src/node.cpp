#include "conduit_core/node.hpp"
#include "conduit_core/exceptions.hpp"
#include "conduit_core/internal/shm_region.hpp"
#include "conduit_core/log.hpp"

#include <chrono>
#include <csignal>

using namespace std::chrono_literals;

namespace conduit {

std::atomic<Node*> Node::active_node_{nullptr};

void Node::signal_handler(int sig) {
    (void)sig;
    if (auto* node = active_node_.load(std::memory_order_relaxed)) {
        node->stop();
    }
}

Node::Node() = default;

Node::~Node() {
    stop();

    for (auto& sub : subscriptions_) {
        if (sub->thread.joinable()) {
            sub->thread.join();
        }
    }

    for (auto& lp : loops_) {
        if (lp->thread.joinable()) {
            lp->thread.join();
        }
    }
}

void Node::install_signal_handlers() {
    active_node_.store(this, std::memory_order_relaxed);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

void Node::uninstall_signal_handlers() {
    std::signal(SIGINT, SIG_DFL);
    std::signal(SIGTERM, SIG_DFL);
    active_node_.store(nullptr, std::memory_order_relaxed);
}

void Node::subscribe(const std::string& topic, std::function<void(const Message&)> callback) {
    if (running_.load(std::memory_order_acquire)) {
        throw NodeError("Cannot subscribe while running");
    }

    auto sub = std::make_unique<Subscription>();
    sub->topic = topic;
    sub->callback = std::move(callback);
    sub->subscriber = nullptr;  // created in run()

    subscriptions_.push_back(std::move(sub));
}

void Node::loop(double rate_hz, std::function<void()> callback) {
    if (running_.load(std::memory_order_acquire)) {
        throw NodeError("Cannot add loop while running");
    }

    if (rate_hz <= 0) {
        throw NodeError("Loop rate must be positive");
    }

    auto lp = std::make_unique<Loop>();
    lp->rate_hz = rate_hz;
    lp->period = std::chrono::nanoseconds(static_cast<int64_t>(1e9 / rate_hz));
    lp->callback = std::move(callback);
    loops_.push_back(std::move(lp));
}

void Node::run() {
    if (running_.load(std::memory_order_acquire)) {
        throw NodeError("Node already running");
    }

    install_signal_handlers();

    running_.store(true, std::memory_order_release);

    // Wait for all topics to exist before creating subscribers
    for (const auto& sub : subscriptions_) {
        if (!internal::ShmRegion::exists(sub->topic)) {
            log::info("Waiting for topic: {}", sub->topic);
            if (!internal::ShmRegion::wait_until_exists(sub->topic, running_)) {
                // Stopped before topic appeared
                uninstall_signal_handlers();
                running_.store(false, std::memory_order_release);
                log::info("Node stopped.");
                return;
            }
        }
    }

    // Create subscribers and start threads
    for (auto& sub : subscriptions_) {
        sub->subscriber = std::make_unique<internal::Subscriber>(sub->topic);
        sub->thread = std::thread(&Node::spin_subscription, this, sub.get());
        log::info("Subscribed to: {}", sub->topic);
    }

    // Start loop threads
    for (auto& lp : loops_) {
        lp->thread = std::thread(&Node::spin_loop, this, lp.get());
        log::info("Started loop: {} Hz", lp->rate_hz);
    }

    log::info("Node running. Press Ctrl+C to stop.");

    // Wait for stop signal
    while (running_.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(100ms);
    }

    // Join subscription threads
    for (auto& sub : subscriptions_) {
        if (sub->thread.joinable()) {
            sub->thread.join();
        }
    }

    // Join loop threads
    for (auto& lp : loops_) {
        if (lp->thread.joinable()) {
            lp->thread.join();
        }
    }

    uninstall_signal_handlers();

    log::info("Node stopped.");
}

void Node::stop() {
    running_.store(false, std::memory_order_release);
}

bool Node::running() const {
    return running_.load(std::memory_order_acquire);
}

void Node::spin_subscription(Subscription* sub) {
    while (running_.load(std::memory_order_acquire)) {
        // Wait with timeout so we can check running_ periodically
        auto msg = sub->subscriber->wait_for(100ms);

        if (msg.has_value()) {
            try {
                sub->callback(*msg);
            } catch (const std::exception& e) {
                log::error("Exception in callback for {}: {}", sub->topic, e.what());
            }
        }
    }
}

void Node::spin_loop(Loop* lp) {
    auto next_time = std::chrono::steady_clock::now();

    while (running_.load(std::memory_order_acquire)) {
        next_time += lp->period;

        try {
            lp->callback();
        } catch (const std::exception& e) {
            log::error("Exception in loop ({} Hz): {}", lp->rate_hz, e.what());
        }

        auto now = std::chrono::steady_clock::now();
        if (next_time > now) {
            std::this_thread::sleep_until(next_time);
        } else {
            log::warn("Loop ({} Hz) missed deadline", lp->rate_hz);
            next_time = now;
        }
    }
}

}  // namespace conduit
