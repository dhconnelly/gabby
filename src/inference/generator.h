#ifndef GABBY_INFERENCE_GENERATOR_H_
#define GABBY_INFERENCE_GENERATOR_H_

#include <filesystem>
#include <memory>
#include <ostream>
#include <string>

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

// TODO: thread-safe
class Generator {
public:
    virtual Message Generate(const Request& req);

    static std::unique_ptr<Generator> LoadFromDirectory(
        std::filesystem::path dir);
};

}  // namespace inference
}  // namespace gabby

#endif  // GABBY_INFERENCE_GENERATOR_H_
