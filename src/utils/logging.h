#ifndef GABBY_UTILS_LOGGING_H_
#define GABBY_UTILS_LOGGING_H_

#include <ostream>
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

#endif  // GABBY_UTILS_LOGGING_H_
