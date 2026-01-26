#pragma once

#include <stdexcept>
#include <string>

namespace conduit {

class ConduitError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ShmError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class RingBufferError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class PublisherError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class SubscriberError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class NodeError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

class TankError : public ConduitError {
public:
    using ConduitError::ConduitError;
};

}  // namespace conduit
