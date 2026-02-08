#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>

namespace conduit {

class WriteBuffer {
public:
    explicit WriteBuffer(uint8_t* data) : ptr_(data) {}

    void write(const std::string& s) {
        uint32_t len = static_cast<uint32_t>(s.size());
        std::memcpy(ptr_, &len, sizeof(len));
        ptr_ += sizeof(len);
        std::memcpy(ptr_, s.data(), s.size());
        ptr_ += s.size();
    }

    template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    void write(const T& val) {
        std::memcpy(ptr_, &val, sizeof(T));
        ptr_ += sizeof(T);
    }

    static size_t size_of(const std::string& s) {
        return sizeof(uint32_t) + s.size();
    }

    template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    static constexpr size_t size_of(const T&) { return sizeof(T); }

private:
    uint8_t* ptr_;
};

class ReadBuffer {
public:
    ReadBuffer(const uint8_t* data, size_t size) : ptr_(data) { (void)size; }

    template<typename T>
    T read() {
        if constexpr (std::is_same_v<T, std::string>) {
            uint32_t len;
            std::memcpy(&len, ptr_, sizeof(len));
            ptr_ += sizeof(len);
            std::string s(reinterpret_cast<const char*>(ptr_), len);
            ptr_ += len;
            return s;
        } else {
            static_assert(std::is_trivially_copyable_v<T>);
            T val;
            std::memcpy(&val, ptr_, sizeof(T));
            ptr_ += sizeof(T);
            return val;
        }
    }

private:
    const uint8_t* ptr_;
};

}  // namespace conduit
