#include "service.h"

#include "http/test_client.h"
#include "http/types.h"
#include "json/json.h"
#include "json/parser.h"
#include "test/test.h"

namespace gabby {

Config kTestConfig{
    .server_config =
        http::ServerConfig{
            .port = 0,
            .read_timeout_millis = 5'000,
            .write_timeout_millis = 10'000,
            .worker_threads = std::thread::hardware_concurrency() - 1,
        },
};

TEST(Service, ChatCompletion) {
    InferenceService service(kTestConfig);
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
    EXPECT_EQ("chat.completion", obj["object"]->as_string().get());

    auto choice = obj["choices"]->as_array()[0]->as_object();
    auto message = choice["message"]->as_object()["content"]->as_string();
    EXPECT_EQ(R"(\n\nhey this is gabby)", message.get());

    service.Stop();
    service.Wait();
}

}  // namespace gabby
