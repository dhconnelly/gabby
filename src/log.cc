#include "log.h"

namespace gabby {
namespace log {

LogLevel GlobalLogLevel = LogLevel::OFF;

void SetGlobalLogLevel(LogLevel level) { GlobalLogLevel = level; }

std::string to_string(LogLevel level) { return std::string(prefix(level)); }

}  // namespace log
}  // namespace gabby
