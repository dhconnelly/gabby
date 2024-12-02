#ifndef GABBY_INFERENCE_GENERATOR_H_
#define GABBY_INFERENCE_GENERATOR_H_

#include <filesystem>
#include <memory>
#include <ostream>
#include <string>

#include "inference/safetensors.h"
#include "json/json.h"

namespace gabby {
namespace inference {

struct Message {
    std::string role;
    std::string content;
};

std::ostream& operator<<(std::ostream& os, const Message& msg);

struct Request {
    Message system_message;
    Message user_message;
};

std::ostream& operator<<(std::ostream& os, const Request& msg);

class Generator {
public:
    virtual ~Generator() = default;
    virtual Message Generate(const Request& req) = 0;
};

class Llama3Generator : public Generator {
public:
    Message Generate(const Request& req) override;

    static std::unique_ptr<Generator> LoadFromDirectory(
        std::filesystem::path dir);

private:
    Llama3Generator(json::ValuePtr config, json::ValuePtr gen_config,
                    json::ValuePtr special_tokens_map,
                    json::ValuePtr tok_config, json::ValuePtr tok,
                    Safetensors tensors)
        : config_(config),
          gen_config_(gen_config),
          special_tokens_map_(special_tokens_map),
          tok_config_(tok_config),
          tok_(tok),
          tensors_(std::move(tensors)) {}
    json::ValuePtr config_;
    json::ValuePtr gen_config_;
    json::ValuePtr special_tokens_map_;
    json::ValuePtr tok_config_;
    json::ValuePtr tok_;
    Safetensors tensors_;
};

}  // namespace inference
}  // namespace gabby

#endif  // GABBY_INFERENCE_GENERATOR_H_
