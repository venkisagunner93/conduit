#include "conduit_tank/tank.hpp"
#include <conduit_core/publisher.hpp>
#include <conduit_core/exceptions.hpp>
#include <conduit_core/internal/shm_region.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstdio>
#include <thread>

using namespace conduit;
using namespace std::chrono_literals;

class TankTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any test topics
        internal::ShmRegion::unlink("test_topic");
        internal::ShmRegion::unlink("topic_a");
        internal::ShmRegion::unlink("topic_b");
        internal::ShmRegion::unlink("topic1");
        internal::ShmRegion::unlink("topic2");
        internal::ShmRegion::unlink("topic");
    }

    bool file_exists(const std::string& path) {
        FILE* f = std::fopen(path.c_str(), "r");
        if (f) {
            std::fclose(f);
            return true;
        }
        return false;
    }
};

TEST_F(TankTest, test_tank_basic) {
    const std::string output_path = "/tmp/test_basic.mcap";
    const std::string topic = "test_topic";

    internal::Publisher pub(topic);

    Tank tank(output_path);
    tank.add_topic(topic);
    tank.start();

    EXPECT_TRUE(tank.recording());

    for (int i = 0; i < 10; ++i) {
        pub.publish(&i, sizeof(i));
        std::this_thread::sleep_for(10ms);
    }

    tank.stop();

    EXPECT_FALSE(tank.recording());
    EXPECT_GE(tank.message_count(), 10u);
    EXPECT_TRUE(file_exists(output_path));

    std::remove(output_path.c_str());
}

TEST_F(TankTest, test_tank_multiple_topics) {
    const std::string output_path = "/tmp/test_multi.mcap";

    internal::Publisher pub_a("topic_a");
    internal::Publisher pub_b("topic_b");

    Tank tank(output_path);
    tank.add_topic("topic_a");
    tank.add_topic("topic_b");
    tank.start();

    for (int i = 0; i < 5; ++i) {
        pub_a.publish("aaa", 3);
        pub_b.publish("bbb", 3);
        std::this_thread::sleep_for(10ms);
    }

    tank.stop();

    EXPECT_GE(tank.message_count(), 10u);

    std::remove(output_path.c_str());
}

TEST_F(TankTest, test_tank_cannot_add_while_recording) {
    internal::Publisher pub("topic1");

    Tank tank("/tmp/test.mcap");
    tank.add_topic("topic1");
    tank.start();

    EXPECT_THROW(tank.add_topic("topic2"), TankError);

    tank.stop();
    std::remove("/tmp/test.mcap");
}

TEST_F(TankTest, test_tank_destructor_stops) {
    internal::Publisher pub("topic");
    const std::string output_path = "/tmp/test_dtor.mcap";

    {
        Tank tank(output_path);
        tank.add_topic("topic");
        tank.start();
        pub.publish("data", 4);
        std::this_thread::sleep_for(50ms);
    }

    // File should exist and be properly closed
    FILE* f = std::fopen(output_path.c_str(), "r");
    EXPECT_NE(f, nullptr);
    if (f) {
        std::fclose(f);
    }

    std::remove(output_path.c_str());
}

TEST_F(TankTest, test_tank_already_recording) {
    internal::Publisher pub("topic");
    const std::string output_path = "/tmp/test_already.mcap";

    Tank tank(output_path);
    tank.add_topic("topic");
    tank.start();

    EXPECT_THROW(tank.start(), TankError);

    tank.stop();
    std::remove(output_path.c_str());
}

TEST_F(TankTest, test_tank_stop_idempotent) {
    internal::Publisher pub("topic");
    const std::string output_path = "/tmp/test_idempotent.mcap";

    Tank tank(output_path);
    tank.add_topic("topic");
    tank.start();
    pub.publish("data", 4);
    std::this_thread::sleep_for(50ms);

    // Multiple stops should be safe
    tank.stop();
    tank.stop();
    tank.stop();

    std::remove(output_path.c_str());
}
