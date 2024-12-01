#include "inference/generator.h"

#include <format>
#include <sstream>

#include "utils/logging.h"

namespace gabby {
namespace inference {

namespace fs = std::filesystem;

std::ostream& operator<<(std::ostream& os, const Message& msg) {
    return os << std::format("{{ \"role\": {}, \"content\": {} }}", msg.role,
                             msg.content);
}

std::string to_string(const Message& msg) {
    std::stringstream ss;
    ss << msg;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Request& msg) {
    return os << std::format("{{ \"system\": {}, \"user\": {} }}",
                             to_string(msg.system_message),
                             to_string(msg.user_message));
}

Message Generator::Generate(const Request& req) {
    return Message{
        .role = "assistant",
        .content = "hey this is gabby, how are u",
    };
}

/* static */ std::unique_ptr<Generator> Generator::LoadFromDirectory(
    std::filesystem::path dir) {
    fs::directory_iterator contents(dir);
    for (fs::path file : contents) {
        LOG(DEBUG) << "scanning: " << file.string();
    }
    return std::make_unique<Generator>();
}

}  // namespace inference
}  // namespace gabby
