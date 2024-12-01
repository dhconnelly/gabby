#include "service.h"

#include "http/test_client.h"
#include "http/types.h"
#include "json/json.h"
#include "json/parser.h"
#include "test/test.h"

namespace gabby {

http::ServerConfig kTestServerConfig = http::ServerConfig{
    .port = 0,
    .read_timeout_millis = 5'000,
    .write_timeout_millis = 10'000,
    .worker_threads = std::thread::hardware_concurrency() - 1,
};

class SimpleGenerator : public inference::Generator {
public:
    inference::Message Generate(const inference::Request& req) override {
        return inference::Message{
            .role = "assistant",
            .content = "this is a test response",
        };
    }
};

TEST(Service, ChatCompletion) {
    InferenceService service(
        std::make_unique<http::HttpServer>(kTestServerConfig),
        std::unique_ptr<inference::Generator>(new SimpleGenerator));
    service.Start();

    json::ValuePtr request = json::Parse(R"({
        "model": "gabby-1",
        "messages": [{
            "role": "system",
            "content": "You are a helpful assistant."
        },{
            "role": "user",
            "content": "Hello!"
        }]
    })");

    json::ValuePtr response =
        http::PostJson(service.port(), "/v1/chat/completions", request);

    auto obj = response->as_object();
    EXPECT_EQ("chat.completion", obj.at("object")->as_string().get());

    auto choice = obj.at("choices")->as_array()[0]->as_object();
    auto message = choice.at("message")->as_object().at("content")->as_string();
    EXPECT_EQ("this is a test response", *message);

    service.Stop();
    service.Wait();
}

}  // namespace gabby
