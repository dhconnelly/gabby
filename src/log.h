#ifndef GABBY_LOG_H_
#define GABBY_LOG_H_

#include <chrono>
#include <format>
#include <sstream>

namespace gabby {

// usage: LOG(INFO) << "hello, world";
#define LOG(level) Logger(__FILE__, __LINE__, level).stream()

enum LogLevel {
    OFF = 0,
    INFO = 1,
    WARN = 2,
    DEBUG = 3,
};

std::ostream& operator<<(std::ostream& os, LogLevel level);

extern LogLevel GlobalLogLevel;
void SetGlobalLogLevel(LogLevel level);

class Logger {
public:
    Logger(const char* filename, int line, LogLevel level);
    ~Logger();
    std::ostream& stream() { return stream_; }

private:
    LogLevel level_;
    std::ostringstream stream_;
};

}  // namespace gabby

#endif  // GABBY_LOG_H_
