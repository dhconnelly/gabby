#ifndef GABBY_INFERENCE_CONFIG_H_
#define GABBY_INFERENCE_CONFIG_H_

#include <filesystem>

#include "inference/safetensors.h"
#include "json/json.h"

namespace gabby {
namespace inference {

struct InferenceConfig {
    json::ValuePtr config;
    json::ValuePtr gen_config;
    json::ValuePtr special_tokens_map;
    json::ValuePtr tok_config;
    json::ValuePtr tok;
    Safetensors tensors;
};

std::unique_ptr<InferenceConfig> LoadConfig(
    const std::filesystem::path& directory);

std::filesystem::path FindDefaultModelDir();

}  // namespace inference
}  // namespace gabby

#endif  // GABBY_INFERENCE_CONFIG_H_
