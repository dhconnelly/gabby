#ifndef GABBY_INFERENCE_GENERATOR_H_
#define GABBY_INFERENCE_GENERATOR_H_

#include <filesystem>
#include <memory>
#include <ostream>
#include <string>

#include "inference/config.h"
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

    static std::unique_ptr<Generator> Load(
        std::unique_ptr<InferenceConfig> config);

private:
    Llama3Generator(std::unique_ptr<InferenceConfig> config)
        : config_(std::move(config)) {}
    std::unique_ptr<InferenceConfig> config_;
};

}  // namespace inference
}  // namespace gabby

#endif  // GABBY_INFERENCE_GENERATOR_H_
