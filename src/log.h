#ifndef GABBY_LOG_H_
#define GABBY_LOG_H_

#include <chrono>
#include <format>
#include <iostream>
#include <string_view>

namespace gabby {
namespace log {

template <typename... Args>
void info(const std::string_view fmt, Args&&... args) {
    auto ts = std::chrono::system_clock::now();
    auto msg =
        std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
    std::cerr << std::format("INFO [{}]: {}\n", ts, msg);
}

}  // namespace log
}  // namespace gabby

#endif  // GABBY_LOG_H_
