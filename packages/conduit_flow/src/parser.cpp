#include "conduit_flow/parser.hpp"
#include <conduit_core/exceptions.hpp>
#include <conduit_core/log.hpp>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <regex>

namespace conduit::flow {

static std::chrono::milliseconds parse_duration(const std::string& str) {
    std::regex re(R"((\d+)\s*(ms|s|m))");
    std::smatch match;

    if (!std::regex_match(str, match, re)) {
        throw ConduitError("Invalid duration: " + str);
    }

    int64_t value = std::stoll(match[1].str());
    std::string unit = match[2].str();

    if (unit == "ms") return std::chrono::milliseconds(value);
    if (unit == "s") return std::chrono::milliseconds(value * 1000);
    if (unit == "m") return std::chrono::milliseconds(value * 60000);

    throw ConduitError("Unknown duration unit: " + unit);
}

static NodeConfig parse_node(const YAML::Node& node) {
    NodeConfig config;

    if (node.IsScalar()) {
        config.name = node.as<std::string>();
        config.exec = config.name;
    } else if (node.IsMap()) {
        if (!node["name"] && !node["exec"]) {
            throw ConduitError("Node must have 'name' or 'exec'");
        }

        if (node["name"]) {
            config.name = node["name"].as<std::string>();
        }

        if (node["exec"]) {
            config.exec = node["exec"].as<std::string>();
        } else {
            config.exec = config.name;
        }

        if (config.name.empty()) {
            config.name = config.exec;
        }

        if (node["args"]) {
            for (const auto& arg : node["args"]) {
                config.args.push_back(arg.as<std::string>());
            }
        }

        if (node["env"]) {
            for (const auto& kv : node["env"]) {
                config.env[kv.first.as<std::string>()] = kv.second.as<std::string>();
            }
        }

        if (node["working_dir"]) {
            config.working_dir = node["working_dir"].as<std::string>();
        }
    } else {
        throw ConduitError("Invalid node format");
    }

    return config;
}

static Step parse_step(const YAML::Node& node) {
    if (node.IsMap() && node["wait"]) {
        auto wait_node = node["wait"];

        if (wait_node.IsScalar()) {
            std::string value = wait_node.as<std::string>();

            if (value.find("topic:") == 0) {
                WaitTopics wt;
                wt.topics.push_back(value.substr(6));
                return wt;
            }

            WaitDuration wd;
            wd.duration = parse_duration(value);
            return wd;
        }

        if (wait_node.IsSequence()) {
            WaitTopics wt;
            for (const auto& item : wait_node) {
                std::string value = item.as<std::string>();
                if (value.find("topic:") == 0) {
                    wt.topics.push_back(value.substr(6));
                } else {
                    throw ConduitError("Wait list must contain topic: entries");
                }
            }
            return wt;
        }
    }

    if (node.IsMap() && node["group"]) {
        Group group;
        for (const auto& item : node["group"]) {
            group.nodes.push_back(parse_node(item));
        }
        return group;
    }

    return parse_node(node);
}

static std::vector<Step> parse_steps(const YAML::Node& node) {
    std::vector<Step> steps;

    if (!node || !node.IsSequence()) {
        return steps;
    }

    for (const auto& item : node) {
        steps.push_back(parse_step(item));
    }

    return steps;
}

static std::vector<Step> reverse_steps(const std::vector<Step>& steps) {
    std::vector<Step> reversed;

    for (auto it = steps.rbegin(); it != steps.rend(); ++it) {
        const Step& step = *it;

        if (std::holds_alternative<WaitDuration>(step) ||
            std::holds_alternative<WaitTopics>(step)) {
            continue;
        }

        reversed.push_back(step);
    }

    return reversed;
}

FlowConfig parse_string(const std::string& yaml) {
    YAML::Node root = YAML::Load(yaml);

    FlowConfig config;

    if (!root["startup"]) {
        throw ConduitError("Flow file must have 'startup' section");
    }

    config.startup = parse_steps(root["startup"]);

    if (root["shutdown"]) {
        config.shutdown = parse_steps(root["shutdown"]);
    } else {
        config.shutdown = reverse_steps(config.startup);
    }

    return config;
}

FlowConfig parse_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw ConduitError("Cannot open flow file: " + path);
    }

    std::string yaml((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

    return parse_string(yaml);
}

}  // namespace conduit::flow
