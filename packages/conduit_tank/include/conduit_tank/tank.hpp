#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace conduit {

/// @brief MCAP-based message recorder with Zstd/LZ4 compression.
///
/// Records messages from one or more topics into an MCAP file.
/// Topics must be added before calling start(). The recorded file
/// can be replayed with the conduit CLI tools.
///
/// @see Node
class Tank {
public:
    /// @brief Construct a recorder targeting the given output file.
    /// @param output_path Path to the MCAP output file.
    explicit Tank(const std::string& output_path);
    ~Tank();

    // No copy, no move
    Tank(const Tank&) = delete;
    Tank& operator=(const Tank&) = delete;
    Tank(Tank&&) = delete;
    Tank& operator=(Tank&&) = delete;

    /// @brief Add a topic to record (must be called before start()).
    /// @param topic Topic name to subscribe to and record.
    void add_topic(const std::string& topic);

    /// @brief Start recording messages from all added topics.
    void start();

    /// @brief Stop recording and finalize the MCAP file.
    void stop();

    /// @brief Check if the recorder is currently active.
    /// @return true if recording is in progress.
    bool recording() const;

    /// @brief Get the total number of messages recorded so far.
    /// @return Message count.
    uint64_t message_count() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace conduit
