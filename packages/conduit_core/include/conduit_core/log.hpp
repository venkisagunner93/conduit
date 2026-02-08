#pragma once

#include <string_view>
#include <fmt/format.h>

/// @brief Conduit logging utilities.
namespace conduit::log {

/// @brief Log severity levels.
enum class Level {
    Debug,   ///< Verbose diagnostic output.
    Info,    ///< Normal operational messages.
    Warn,    ///< Potential issues that do not prevent operation.
    Error    ///< Errors that may prevent correct operation.
};

/// @brief Set the global log level. Messages below this level are suppressed.
/// @param level The minimum severity level to output.
void set_level(Level level);

/// @brief Get the current global log level.
/// @return The active log level.
Level get_level();

/// @brief Log a debug message.
/// @param msg Message string.
void debug(std::string_view msg);

/// @brief Log an informational message.
/// @param msg Message string.
void info(std::string_view msg);

/// @brief Log a warning message.
/// @param msg Message string.
void warn(std::string_view msg);

/// @brief Log an error message.
/// @param msg Message string.
void error(std::string_view msg);

/// @brief Log a formatted debug message.
/// @tparam Args Format argument types.
/// @param fmt Format string (fmt library syntax).
/// @param args Format arguments.
template<typename... Args>
void debug(fmt::format_string<Args...> fmt, Args&&... args) {
    debug(fmt::format(fmt, std::forward<Args>(args)...));
}

/// @brief Log a formatted informational message.
/// @tparam Args Format argument types.
/// @param fmt Format string (fmt library syntax).
/// @param args Format arguments.
template<typename... Args>
void info(fmt::format_string<Args...> fmt, Args&&... args) {
    info(fmt::format(fmt, std::forward<Args>(args)...));
}

/// @brief Log a formatted warning message.
/// @tparam Args Format argument types.
/// @param fmt Format string (fmt library syntax).
/// @param args Format arguments.
template<typename... Args>
void warn(fmt::format_string<Args...> fmt, Args&&... args) {
    warn(fmt::format(fmt, std::forward<Args>(args)...));
}

/// @brief Log a formatted error message.
/// @tparam Args Format argument types.
/// @param fmt Format string (fmt library syntax).
/// @param args Format arguments.
template<typename... Args>
void error(fmt::format_string<Args...> fmt, Args&&... args) {
    error(fmt::format(fmt, std::forward<Args>(args)...));
}

}  // namespace conduit::log
