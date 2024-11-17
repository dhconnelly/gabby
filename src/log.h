#ifndef GABBY_LOG_H_
#define GABBY_LOG_H_

#include <chrono>
#include <format>
#include <iostream>
#include <string_view>

namespace gabby {
namespace log {

enum class LogLevel : int {
    OFF = 0,
    INFO = 1,
    WARN = 2,
    DEBUG = 3,
};

extern LogLevel GlobalLogLevel;

void SetGlobalLogLevel(LogLevel level);

constexpr std::string_view prefix(LogLevel level) {
    switch (level) {
        case LogLevel::OFF: return "<logging disabled>";
        case LogLevel::INFO: return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::WARN: return "WARN";
    }
}

std::string to_string(LogLevel level);

template <typename... Args>
void log(LogLevel level, const std::string_view fmt, Args... args) {
    if (level > GlobalLogLevel) return;
    auto pfx = prefix(level);
    auto ts = std::chrono::system_clock::now();
    auto msg = std::vformat(fmt, std::make_format_args(args...));
    std::cerr << std::format("{} [{}]: {}\n", pfx, ts, msg);
}

template <typename... Args>
void info(const std::string_view fmt, Args... args) {
    log(LogLevel::INFO, fmt, args...);
}

template <typename... Args>
void debug(const std::string_view fmt, Args... args) {
    log(LogLevel::DEBUG, fmt, args...);
}

template <typename... Args>
void warn(const std::string_view fmt, Args... args) {
    log(LogLevel::WARN, fmt, args...);
}

}  // namespace log
}  // namespace gabby

#endif  // GABBY_LOG_H_
