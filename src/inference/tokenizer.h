#ifndef GABBY_INFERENCE_TOKENIZER_H_
#define GABBY_INFERENCE_TOKENIZER_H_

#include <memory>

#include "json/json.h"

namespace gabby {
namespace inference {

class Tokenizer {
public:
    virtual ~Tokenizer() = default;

    Tokenizer(json::ValuePtr special_tokens_map,
              json::ValuePtr tokenizer_config, json::ValuePtr tokens);

    virtual std::vector<int> Tokenize(const std::string_view input);

private:
};

}  // namespace inference
}  // namespace gabby

#endif  // GABBY_INFERENCE_TOKENIZER_H_
