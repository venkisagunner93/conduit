#pragma once

#include <stdexcept>
#include <string>

namespace conduit {

/// @brief Base exception for all conduit errors.
class ConduitError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// @brief Error during shared memory operations (create, open, map).
class ShmError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

/// @brief Error in ring buffer operations (invalid config, overwrite detected).
class RingBufferError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

/// @brief Error during publisher creation or publishing.
class PublisherError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

/// @brief Error during subscriber creation or message reading.
class SubscriberError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

/// @brief Error in Node lifecycle (run, stop, signal handling).
class NodeError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

/// @brief Error during MCAP recording (file I/O, codec failures).
class TankError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

}  // namespace conduit
