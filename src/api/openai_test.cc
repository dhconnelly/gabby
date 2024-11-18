#include "api/openai.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "http/types.h"

namespace gabby {
namespace api {

struct Response {
    http::StatusCode code;
    std::string response;  // TODO: json
};

struct SimpleResponseWriter : public http::ResponseWriter {
    void SendStatus(http::StatusCode code) override { this->code = code; }
    void Send(std::string_view data) override { this->data = data; }

    http::StatusCode code;
    std::string data;
};

Response call(http::Handler h, http::Request&& req) {
    SimpleResponseWriter resp;
    h(req, resp);
    return Response{.code = resp.code, .response = resp.data};
}

TEST(OpenAIAPI, TestInvalidRequest) {
    auto api = BuildOpenAIAPI();
    auto resp = call(api, http::Request{
                              .path = "/v1/chat/completions",
                              .params = {{"foo", "bar"}},
                          });
    EXPECT_EQ(http::StatusCode::BadRequest, resp.code);
}

TEST(OpenAIAPI, TestInvalidModel) {
    //
    EXPECT_EQ(1, 1);
}

TEST(OpenAIAPI, TestCompletion) {
    //
    EXPECT_EQ(1, 1);
}

}  // namespace api
}  // namespace gabby
