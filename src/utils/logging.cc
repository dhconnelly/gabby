#include "utils/logging.h"

#include <chrono>
#include <format>
#include <iostream>

namespace gabby {

LogLevel GlobalLogLevel = LogLevel::OFF;

void SetGlobalLogLevel(LogLevel level) { GlobalLogLevel = level; }

std::ostream &operator<<(std::ostream &os, LogLevel level) {
    switch (level) {
        case LogLevel::OFF: return os << "OFF";
        case LogLevel::ERROR: return os << "ERROR";
        case LogLevel::INFO: return os << "INFO";
        case LogLevel::DEBUG: return os << "DEBUG";
        case LogLevel::WARN: return os << "WARN";
    }
}

const char *basename(const char *filename) {
    const char *slash = strrchr(filename, '/');
    return slash ? slash + 1 : filename;
}

Logger::Logger(const char *filename, int line, LogLevel level) : level_(level) {
    auto ts = std::chrono::system_clock::now();
    stream_ << std::format("{}", ts) << " " << basename(filename) << ":" << line
            << " " << level << ": ";
}

Logger::~Logger() {
    stream_ << std::endl;
    std::cerr << stream_.str();
}

}  // namespace gabby
