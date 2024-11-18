#include "api/openai.h"

#include <string>
#include <variant>
#include <vector>

#include "http/router.h"
#include "log.h"

namespace gabby {
namespace api {

struct Context {};

struct Message {
    std::string role;
    std::string content;
};

struct CompletionRequest {
    std::string model;
    std::vector<Message> messages;
};

struct Choice {
    std::string role;
    std::string message;
};

struct CompletionResponse {
    std::string model;
    std::vector<Choice> choice;
};

struct CompletionError {
    std::string reason;
};

using CompletionStatus = std::variant<CompletionError, CompletionResponse>;

CompletionStatus Complete(Context& ctx, const CompletionRequest& request) {
    return CompletionError{.reason = "unimplemented"};
}

std::optional<CompletionRequest> ParseRequest(Context& ctx,
                                              const http::Request& req) {
    return {};
}

http::Handler BuildOpenAIAPI() {
    std::shared_ptr<Context> ctx(new Context);
    return http::Router::builder()
        .route("/v1/chat/completions",
               [ctx](const http::Request& request,
                     http::ResponseWriter& response) {
                   std::optional<CompletionRequest> parsed =
                       ParseRequest(*ctx, request);
                   if (!parsed) {
                       return response.SendStatus(http::StatusCode::BadRequest);
                   }
                   response.SendStatus(http::StatusCode::OK);
               })
        .build();
}

}  // namespace api
}  // namespace gabby
