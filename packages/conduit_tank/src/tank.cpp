#include "conduit_tank/tank.hpp"
#include <conduit_core/subscriber.hpp>
#include <conduit_core/exceptions.hpp>

#define MCAP_IMPLEMENTATION
#include <mcap/writer.hpp>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

namespace conduit {

struct TopicRecorder {
    std::string topic;
    std::unique_ptr<Subscriber> subscriber;
    std::thread thread;
    mcap::ChannelId channel_id;
};

struct Tank::Impl {
    std::string output_path;
    std::vector<std::unique_ptr<TopicRecorder>> topic_recorders;
    std::atomic<bool> running{false};
    std::atomic<uint64_t> message_count{0};

    mcap::McapWriter writer;
    std::mutex write_mutex;

    void record_loop(TopicRecorder* tr);
};

Tank::Tank(const std::string& output_path)
    : impl_(std::make_unique<Impl>()) {
    impl_->output_path = output_path;
}

Tank::~Tank() {
    if (impl_->running) {
        stop();
    }
}

void Tank::add_topic(const std::string& topic) {
    if (impl_->running) {
        throw TankError("Cannot add topic while recording");
    }

    auto tr = std::make_unique<TopicRecorder>();
    tr->topic = topic;
    impl_->topic_recorders.push_back(std::move(tr));
}

void Tank::start() {
    if (impl_->running) {
        throw TankError("Already recording");
    }

    // Open MCAP file
    mcap::McapWriterOptions options("");
    options.compression = mcap::Compression::Zstd;

    auto status = impl_->writer.open(impl_->output_path, options);
    if (!status.ok()) {
        throw TankError("Failed to open MCAP: " + status.message);
    }

    // Create channels (no schema - raw bytes)
    for (auto& tr : impl_->topic_recorders) {
        mcap::Channel channel;
        channel.topic = tr->topic;
        channel.messageEncoding = "";
        channel.schemaId = 0;

        impl_->writer.addChannel(channel);
        tr->channel_id = channel.id;
    }

    impl_->running = true;

    // Start threads
    for (auto& tr : impl_->topic_recorders) {
        tr->subscriber = std::make_unique<Subscriber>(tr->topic);
        tr->thread = std::thread([this, tr_ptr = tr.get()]() {
            impl_->record_loop(tr_ptr);
        });
    }
}

void Tank::stop() {
    if (!impl_->running) {
        return;
    }

    impl_->running = false;

    // Join threads
    for (auto& tr : impl_->topic_recorders) {
        if (tr->thread.joinable()) {
            tr->thread.join();
        }
    }

    // Close file
    impl_->writer.close();
}

bool Tank::recording() const {
    return impl_->running.load();
}

uint64_t Tank::message_count() const {
    return impl_->message_count.load();
}

void Tank::Impl::record_loop(TopicRecorder* tr) {
    using namespace std::chrono_literals;

    while (running) {
        auto msg = tr->subscriber->wait_for(100ms);

        if (msg.has_value()) {
            mcap::Message mcap_msg;
            mcap_msg.channelId = tr->channel_id;
            mcap_msg.publishTime = msg->timestamp_ns;
            mcap_msg.logTime = msg->timestamp_ns;
            mcap_msg.sequence = static_cast<uint32_t>(msg->sequence);
            mcap_msg.data = reinterpret_cast<const std::byte*>(msg->data);
            mcap_msg.dataSize = msg->size;

            {
                std::lock_guard<std::mutex> lock(write_mutex);
                (void)writer.write(mcap_msg);
            }

            message_count.fetch_add(1);
        }
    }
}

}  // namespace conduit
