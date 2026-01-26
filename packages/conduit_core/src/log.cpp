#include "conduit_core/log.hpp"
#include <fmt/chrono.h>
#include <chrono>
#include <cstdio>
#include <mutex>

namespace conduit::log {

static Level g_level = Level::Info;
static std::mutex g_mutex;

void set_level(Level level) {
    g_level = level;
}

Level get_level() {
    return g_level;
}

static const char* level_str(Level level) {
    switch (level) {
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO ";
        case Level::Warn:  return "WARN ";
        case Level::Error: return "ERROR";
    }
    return "?????";
}

static const char* level_color(Level level) {
    switch (level) {
        case Level::Debug: return "\033[36m";
        case Level::Info:  return "\033[32m";
        case Level::Warn:  return "\033[33m";
        case Level::Error: return "\033[31m";
    }
    return "";
}

static void log_impl(Level level, std::string_view msg) {
    if (level < g_level) {
        return;
    }

    auto now = std::chrono::system_clock::now();

    std::lock_guard<std::mutex> lock(g_mutex);
    fmt::print(stderr, "{}[{}] [{}]\033[0m {}\n",
        level_color(level),
        now,
        level_str(level),
        msg
    );
}

void debug(std::string_view msg) { log_impl(Level::Debug, msg); }
void info(std::string_view msg)  { log_impl(Level::Info, msg); }
void warn(std::string_view msg)  { log_impl(Level::Warn, msg); }
void error(std::string_view msg) { log_impl(Level::Error, msg); }

}  // namespace conduit::log
