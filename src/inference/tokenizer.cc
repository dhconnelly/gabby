#include "inference/tokenizer.h"

namespace gabby {
namespace inference {

std::vector<int> Tokenizer::Tokenize(const std::string_view input) {
    return {};
}

Tokenizer::Tokenizer(json::ValuePtr special_tokens_map,
                     json::ValuePtr tokenizer_config, json::ValuePtr tokens) {}

}  // namespace inference
}  // namespace gabby
