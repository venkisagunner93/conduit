#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace conduit {

class Tank {
public:
    explicit Tank(const std::string& output_path);
    ~Tank();

    // No copy, no move
    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;
    Tank(Tank&&) = delete;
    Tank& operator=(Tank&&) = delete;

    // Add topic to record (call before start)
    void add_topic(const std::string& topic);

    // Start recording
    void start();

    // Stop recording and finalize file
    void stop();

    // Check if recording
    bool recording() const;

    // Get number of messages recorded
    uint64_t message_count() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace conduit
