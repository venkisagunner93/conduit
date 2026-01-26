#include <conduit_flow/flow.hpp>
#include <conduit_flow/parser.hpp>
#include <gtest/gtest.h>

using namespace conduit::flow;

TEST(FlowParser, SimpleNode) {
    auto config = parse_string(R"(
startup:
  - my_node
)");

    ASSERT_EQ(config.startup.size(), 1);
    ASSERT_TRUE(std::holds_alternative<NodeConfig>(config.startup[0]));

    auto& node = std::get<NodeConfig>(config.startup[0]);
    EXPECT_EQ(node.name, "my_node");
    EXPECT_EQ(node.exec, "my_node");
}

TEST(FlowParser, NodeWithOptions) {
    auto config = parse_string(R"(
startup:
  - name: my_node
    exec: my_executable
    args: ["--flag", "value"]
    env:
      MY_VAR: "123"
    working_dir: /tmp
)");

    ASSERT_EQ(config.startup.size(), 1);
    auto& node = std::get<NodeConfig>(config.startup[0]);

    EXPECT_EQ(node.name, "my_node");
    EXPECT_EQ(node.exec, "my_executable");
    ASSERT_EQ(node.args.size(), 2);
    EXPECT_EQ(node.args[0], "--flag");
    EXPECT_EQ(node.args[1], "value");
    EXPECT_EQ(node.env.at("MY_VAR"), "123");
    EXPECT_EQ(node.working_dir, "/tmp");
}

TEST(FlowParser, WaitDuration) {
    auto config = parse_string(R"(
startup:
  - wait: 500ms
  - wait: 2s
  - wait: 1m
)");

    ASSERT_EQ(config.startup.size(), 3);

    auto& w1 = std::get<WaitDuration>(config.startup[0]);
    EXPECT_EQ(w1.duration.count(), 500);

    auto& w2 = std::get<WaitDuration>(config.startup[1]);
    EXPECT_EQ(w2.duration.count(), 2000);

    auto& w3 = std::get<WaitDuration>(config.startup[2]);
    EXPECT_EQ(w3.duration.count(), 60000);
}

TEST(FlowParser, WaitTopic) {
    auto config = parse_string(R"(
startup:
  - wait: topic:my_topic
)");

    ASSERT_EQ(config.startup.size(), 1);
    auto& wt = std::get<WaitTopics>(config.startup[0]);
    ASSERT_EQ(wt.topics.size(), 1);
    EXPECT_EQ(wt.topics[0], "my_topic");
}

TEST(FlowParser, WaitMultipleTopics) {
    auto config = parse_string(R"(
startup:
  - wait:
      - topic:topic_a
      - topic:topic_b
)");

    ASSERT_EQ(config.startup.size(), 1);
    auto& wt = std::get<WaitTopics>(config.startup[0]);
    ASSERT_EQ(wt.topics.size(), 2);
    EXPECT_EQ(wt.topics[0], "topic_a");
    EXPECT_EQ(wt.topics[1], "topic_b");
}

TEST(FlowParser, Group) {
    auto config = parse_string(R"(
startup:
  - group:
      - node_a
      - node_b
      - name: node_c
        exec: node_c_exec
)");

    ASSERT_EQ(config.startup.size(), 1);
    auto& group = std::get<Group>(config.startup[0]);
    ASSERT_EQ(group.nodes.size(), 3);
    EXPECT_EQ(group.nodes[0].name, "node_a");
    EXPECT_EQ(group.nodes[1].name, "node_b");
    EXPECT_EQ(group.nodes[2].name, "node_c");
    EXPECT_EQ(group.nodes[2].exec, "node_c_exec");
}

TEST(FlowParser, ShutdownDefault) {
    auto config = parse_string(R"(
startup:
  - node_a
  - node_b
  - node_c
)");

    ASSERT_EQ(config.shutdown.size(), 3);
    EXPECT_EQ(std::get<NodeConfig>(config.shutdown[0]).name, "node_c");
    EXPECT_EQ(std::get<NodeConfig>(config.shutdown[1]).name, "node_b");
    EXPECT_EQ(std::get<NodeConfig>(config.shutdown[2]).name, "node_a");
}

TEST(FlowParser, ShutdownExplicit) {
    auto config = parse_string(R"(
startup:
  - node_a
  - node_b

shutdown:
  - node_a
  - wait: 1s
  - node_b
)");

    ASSERT_EQ(config.shutdown.size(), 3);
    EXPECT_EQ(std::get<NodeConfig>(config.shutdown[0]).name, "node_a");
    EXPECT_EQ(std::get<WaitDuration>(config.shutdown[1]).duration.count(), 1000);
    EXPECT_EQ(std::get<NodeConfig>(config.shutdown[2]).name, "node_b");
}

TEST(FlowParser, FullExample) {
    auto config = parse_string(R"(
startup:
  - group:
      - lidar_driver
      - camera_driver
  - wait:
      - topic:lidar_points
      - topic:camera_image
  - perception_node
  - wait: topic:detections
  - planning_node
  - control_node
)");

    ASSERT_EQ(config.startup.size(), 6);

    auto& group = std::get<Group>(config.startup[0]);
    EXPECT_EQ(group.nodes.size(), 2);

    auto& wt = std::get<WaitTopics>(config.startup[1]);
    EXPECT_EQ(wt.topics.size(), 2);

    EXPECT_EQ(std::get<NodeConfig>(config.startup[2]).name, "perception_node");
    EXPECT_EQ(std::get<WaitTopics>(config.startup[3]).topics[0], "detections");
    EXPECT_EQ(std::get<NodeConfig>(config.startup[4]).name, "planning_node");
    EXPECT_EQ(std::get<NodeConfig>(config.startup[5]).name, "control_node");
}
