#include "conduit_core/node.hpp"
#include "conduit_core/exceptions.hpp"
#include "conduit_core/internal/shm_region.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <optional>
#include <thread>

using namespace conduit;
using namespace std::chrono_literals;

class NodeTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any test topics
        internal::ShmRegion::unlink("test_topic");
        internal::ShmRegion::unlink("imu");
        internal::ShmRegion::unlink("lidar");
        internal::ShmRegion::unlink("input");
        internal::ShmRegion::unlink("output");
        internal::ShmRegion::unlink("never_publishes");
        internal::ShmRegion::unlink("dummy");
        internal::ShmRegion::unlink("late");
    }
};

TEST_F(NodeTest, test_node_basic) {
    class TestNode : public Node {
    public:
        std::atomic<int> count{0};

        TestNode() {
            subscribe("test_topic", &TestNode::on_message);
        }

        void on_message(const Message& msg) {
            (void)msg;
            count.fetch_add(1, std::memory_order_release);
        }
    };

    // Create publisher first
    Publisher pub("test_topic");

    // Create and run node in separate thread
    TestNode node;
    std::thread node_thread([&node]() {
        node.run();
    });

    // Wait for node to start
    std::this_thread::sleep_for(50ms);

    // Publish messages
    pub.publish("hello", 5);
    pub.publish("world", 5);

    // Wait for processing
    std::this_thread::sleep_for(50ms);

    // Stop node
    node.stop();
    node_thread.join();

    EXPECT_GE(node.count.load(std::memory_order_acquire), 2);
}

TEST_F(NodeTest, test_node_multiple_subscriptions) {
    class TestNode : public Node {
    public:
        std::atomic<int> imu_count{0};
        std::atomic<int> lidar_count{0};

        TestNode() {
            subscribe("imu", &TestNode::on_imu);
            subscribe("lidar", &TestNode::on_lidar);
        }

        void on_imu(const Message& msg) {
            (void)msg;
            imu_count.fetch_add(1, std::memory_order_release);
        }

        void on_lidar(const Message& msg) {
            (void)msg;
            lidar_count.fetch_add(1, std::memory_order_release);
        }
    };

    Publisher pub_imu("imu");
    Publisher pub_lidar("lidar");

    TestNode node;
    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);

    pub_imu.publish("data", 4);
    pub_imu.publish("data", 4);
    pub_lidar.publish("data", 4);

    std::this_thread::sleep_for(50ms);

    node.stop();
    node_thread.join();

    EXPECT_GE(node.imu_count.load(std::memory_order_acquire), 2);
    EXPECT_GE(node.lidar_count.load(std::memory_order_acquire), 1);
}

TEST_F(NodeTest, test_node_advertise_and_publish) {
    class ProducerNode : public Node {
    public:
        std::optional<Publisher> output_;

        ProducerNode() {
            subscribe("input", &ProducerNode::on_input);
            output_.emplace(advertise("output"));
        }

        void on_input(const Message& msg) {
            output_->publish(msg.data, msg.size);
        }
    };

    Publisher pub_input("input");

    ProducerNode node;

    // Create subscriber after ProducerNode's advertise() creates the output topic
    Subscriber sub_output("output");

    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);

    pub_input.publish("passthrough", 11);

    std::this_thread::sleep_for(50ms);

    auto msg = sub_output.take();
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->size, 11u);

    node.stop();
    node_thread.join();
}

TEST_F(NodeTest, test_node_stop_while_waiting) {
    class TestNode : public Node {
    public:
        TestNode() {
            subscribe("never_publishes", &TestNode::on_message);
        }

        void on_message(const Message& msg) {
            (void)msg;
        }
    };

    // Create publisher so topic exists
    Publisher pub("never_publishes");

    TestNode node;
    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);

    // Node should be blocked waiting
    // Stop should unblock it
    auto start = std::chrono::steady_clock::now();
    node.stop();
    node_thread.join();
    auto elapsed = std::chrono::steady_clock::now() - start;

    // Should stop within ~100ms (wait_for timeout)
    EXPECT_LT(elapsed, 200ms);
}

TEST_F(NodeTest, test_node_lambda_subscribe) {
    std::atomic<int> count{0};

    class TestNode : public Node {
    public:
        TestNode(std::atomic<int>& count_ref) {
            subscribe("test_topic", [&count_ref](const Message& msg) {
                (void)msg;
                count_ref.fetch_add(1, std::memory_order_release);
            });
        }
    };

    Publisher pub("test_topic");

    TestNode node(count);
    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);

    pub.publish("data", 4);

    std::this_thread::sleep_for(50ms);

    node.stop();
    node_thread.join();

    EXPECT_GE(count.load(std::memory_order_acquire), 1);
}

TEST_F(NodeTest, test_node_cannot_subscribe_while_running) {
    class TestNode : public Node {
    public:
        void late_subscribe() {
            subscribe("late", [](const Message&) {});
        }
    };

    Publisher pub("dummy");

    TestNode node;
    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);

    EXPECT_THROW(node.late_subscribe(), NodeError);

    node.stop();
    node_thread.join();
}

TEST_F(NodeTest, test_node_running_state) {
    class TestNode : public Node {
    public:
        TestNode() {
            subscribe("test_topic", &TestNode::on_message);
        }

        void on_message(const Message& msg) {
            (void)msg;
        }
    };

    Publisher pub("test_topic");

    TestNode node;
    EXPECT_FALSE(node.running());

    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);
    EXPECT_TRUE(node.running());

    node.stop();
    node_thread.join();

    EXPECT_FALSE(node.running());
}

TEST_F(NodeTest, test_node_cannot_run_twice) {
    class TestNode : public Node {
    public:
        TestNode() {
            subscribe("test_topic", &TestNode::on_message);
        }

        void on_message(const Message& msg) {
            (void)msg;
        }
    };

    Publisher pub("test_topic");

    TestNode node;
    std::thread node_thread([&node]() {
        node.run();
    });

    std::this_thread::sleep_for(50ms);

    // Trying to run again should throw
    EXPECT_THROW(node.run(), NodeError);

    node.stop();
    node_thread.join();
}
