#include "log.h"

#include <chrono>
#include <format>
#include <iostream>

namespace gabby {
namespace log {

void info(const std::string_view message) {
    auto ts = std::chrono::system_clock::now();
    std::cerr << std::format("INFO [{}]: {}\n", ts, message);
}

}  // namespace log
}  // namespace gabby
