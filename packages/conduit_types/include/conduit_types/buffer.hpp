#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>

namespace conduit {

/// @brief Sequential binary writer for serializing message fields.
///
/// Writes values contiguously into a pre-allocated byte buffer. Strings
/// are length-prefixed (uint32_t). Trivially copyable types are written
/// via memcpy.
///
/// @see ReadBuffer
class WriteBuffer {
public:
    /// @brief Construct a write buffer starting at the given pointer.
    /// @param data Pointer to the output buffer.
    explicit WriteBuffer(uint8_t* data) : ptr_(data) {}

    /// @brief Write a length-prefixed string.
    /// @param s The string to write.
    void write(const std::string& s) {
        uint32_t len = static_cast<uint32_t>(s.size());
        std::memcpy(ptr_, &len, sizeof(len));
        ptr_ += sizeof(len);
        std::memcpy(ptr_, s.data(), s.size());
        ptr_ += s.size();
    }

    /// @brief Write a trivially copyable value.
    /// @tparam T Value type (must be trivially copyable).
    /// @param val The value to write.
    template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    void write(const T& val) {
        std::memcpy(ptr_, &val, sizeof(T));
        ptr_ += sizeof(T);
    }

    /// @brief Compute the serialized size of a string (4-byte length prefix + content).
    /// @param s The string.
    /// @return Size in bytes.
    static size_t size_of(const std::string& s) {
        return sizeof(uint32_t) + s.size();
    }

    /// @brief Compute the serialized size of a trivially copyable value.
    /// @tparam T Value type.
    /// @return sizeof(T).
    template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
    static constexpr size_t size_of(const T&) { return sizeof(T); }

private:
    uint8_t* ptr_;
};

/// @brief Sequential binary reader for deserializing message fields.
///
/// Reads values contiguously from a byte buffer. Strings are read as
/// length-prefixed (uint32_t). Trivially copyable types are read via memcpy.
///
/// @see WriteBuffer
class ReadBuffer {
public:
    /// @brief Construct a read buffer over the given data.
    /// @param data Pointer to the input buffer.
    /// @param size Total buffer size in bytes (used for bounds context).
    ReadBuffer(const uint8_t* data, size_t size) : ptr_(data) { (void)size; }

    /// @brief Read the next value from the buffer.
    ///
    /// For std::string, reads a uint32_t length prefix followed by that many
    /// characters. For trivially copyable types, reads sizeof(T) bytes.
    ///
    /// @tparam T Type to read (std::string or trivially copyable).
    /// @return The deserialized value.
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
