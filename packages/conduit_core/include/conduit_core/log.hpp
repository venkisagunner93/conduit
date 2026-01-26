#pragma once

#include <string_view>
#include <fmt/format.h>

namespace conduit::log {

enum class Level {
    Debug,
    Info,
    Warn,
    Error
};

void set_level(Level level);
Level get_level();

void debug(std::string_view msg);
void info(std::string_view msg);
void warn(std::string_view msg);
void error(std::string_view msg);

template<typename... Args>
void debug(fmt::format_string<Args...> fmt, Args&&... args) {
    debug(fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void info(fmt::format_string<Args...> fmt, Args&&... args) {
    info(fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void warn(fmt::format_string<Args...> fmt, Args&&... args) {
    warn(fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void error(fmt::format_string<Args...> fmt, Args&&... args) {
    error(fmt::format(fmt, std::forward<Args>(args)...));
}

}  // namespace conduit::log
