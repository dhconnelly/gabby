#ifndef GABBY_UTILS_LOGGING_H_
#define GABBY_UTILS_LOGGING_H_

#include <cstring>
#include <format>
#include <ostream>
#include <source_location>
#include <sstream>

namespace gabby {

// usage: LOG(INFO) << "hello, world";
#define LOG(level) gabby::Logger(__FILE__, __LINE__, level).stream()

enum LogLevel {
    OFF = 0,
    ERROR = 1,
    INFO = 2,
    WARN = 3,
    DEBUG = 4,
};

std::ostream& operator<<(std::ostream& os, LogLevel level);

extern LogLevel GlobalLogLevel;
void SetGlobalLogLevel(LogLevel level);

class ScopedLogLevel {
public:
    ScopedLogLevel(LogLevel level) : prev_(GlobalLogLevel) {
        GlobalLogLevel = level;
    }
    ~ScopedLogLevel() { GlobalLogLevel = prev_; }

private:
    LogLevel prev_;
};

class Logger {
public:
    Logger(const char* filename, int line, LogLevel level);
    ~Logger();
    std::ostream& stream() { return stream_; }

private:
    LogLevel level_;
    std::ostringstream stream_;
};

class SystemError : public std::exception {
public:
    SystemError(int error,
                std::source_location where = std::source_location::current())
        : errno_(error),
          where_(where),
          what_(std::format("{}:{}: {}", where_.file_name(), where_.line(),
                            strerror(error))) {}
    const char* what() const noexcept override { return what_.c_str(); }
    int error() const { return errno_; }

private:
    int errno_;
    std::source_location where_;
    std::string what_;
};

}  // namespace gabby

#endif  // GABBY_UTILS_LOGGING_H_
