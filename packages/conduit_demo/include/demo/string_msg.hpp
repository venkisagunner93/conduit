#pragma once

#include <conduit_types/buffer.hpp>
#include <conduit_types/variable_message_type.hpp>
#include <string>

struct StringMsg : public conduit::VariableMessageType {
    std::string text;

    StringMsg() = default;
    explicit StringMsg(std::string t) : text(std::move(t)) {}

    size_t serialized_size() const override {
        return conduit::WriteBuffer::size_of(text);
    }

    void serialize(uint8_t* buffer) const override {
        conduit::WriteBuffer(buffer).write(text);
    }

    static StringMsg deserialize(const uint8_t* data, size_t size) {
        return StringMsg(conduit::ReadBuffer(data, size).read<std::string>());
    }
};
