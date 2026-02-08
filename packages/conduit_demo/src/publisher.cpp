#include <conduit_core/log.hpp>
#include <conduit_core/node.hpp>
#include <conduit_types/derived/odometry.hpp>
#include <conduit_types/primitives/uint.hpp>
#include <demo/string_msg.hpp>
#include <optional>

using namespace conduit;

class HelloPublisher : public Node {
public:
    HelloPublisher() {
        hello_.emplace(advertise<StringMsg>("hello"));
        counter_.emplace(advertise<Uint>("counter"));
        odometry_.emplace(advertise<Odometry>("odometry"));
        loop(1.0, &HelloPublisher::publish);
    }

private:
    void publish() {
        StringMsg msg(fmt::format("Hello World #{}", count_));
        hello_->publish(msg);
        log::info("Published: {}", msg.text);

        Uint u{};
        u.value = count_;
        counter_->publish(u);

        Odometry odom{};
        set_frame(odom.header.frame, "odom");
        set_frame(odom.child_frame, "base_link");
        odom.pose.position.x = static_cast<double>(count_);
        odom.pose.orientation = Orientation::from_yaw(0.1 * count_);
        odom.linear_velocity.x = 1.0;
        odometry_->publish(odom);

        log::info("Orientation published: {}", odom.pose.orientation.to_yaw());

        ++count_;
    }

    std::optional<Publisher<StringMsg>> hello_;
    std::optional<Publisher<Uint>> counter_;
    std::optional<Publisher<Odometry>> odometry_;
    uint64_t count_ = 0;
};

int main() {
    HelloPublisher node;
    node.run();
    return 0;
}
