#include "inference/generator.h"

#include <cerrno>
#include <cstdio>
#include <format>
#include <sstream>

#include "json/parser.h"
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

Message Llama3Generator::Generate(const Request& req) {
    return Message{
        .role = "assistant",
        .content = "hey this is gabby, how are u",
    };
}

/* static */
std::unique_ptr<Generator> Llama3Generator::LoadFromDirectory(
    std::filesystem::path dir) {
    auto config = json::ParseFile(dir / "config.json");
    auto gen_config = json::ParseFile(dir / "generation_config.json");
    auto special_tokens_map = json::ParseFile(dir / "special_tokens_map.json");
    auto tok_config = json::ParseFile(dir / "tokenizer_config.json");
    auto tok = json::ParseFile(dir / "tokenizer.json");
    auto tensors = Safetensors::LoadFile(dir / "model.safetensors");
    return std::unique_ptr<Generator>(
        new Llama3Generator(config, gen_config, special_tokens_map, tok_config,
                            tok, std::move(tensors)));
}

}  // namespace inference
}  // namespace gabby
