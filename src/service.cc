#include "service.h"

#include <cstdio>
#include <format>
#include <string>
#include <unordered_set>

#include "http/router.h"
#include "json/json.h"
#include "json/parser.h"
#include "utils/logging.h"

namespace gabby {

namespace {
std::string ReadAll(FILE* stream) {
    std::string data;
    char buf[1024];
    int n;
    while ((n = fread(buf, 1, 1024, stream)) > 0) {
        data.append(buf, n);
    }
    if (n < 0) throw SystemError(errno);
    return data;
}

const std::string& GetHeader(const http::Request& req, const std::string& key) {
    auto it = req.headers.find(key);
    if (it == req.headers.end()) {
        throw http::BadRequestException("missing header: " + key);
    }
    return it->second;
}

int GetIntHeader(const http::Request& req, const std::string& key) {
    const std::string& value = GetHeader(req, key);
    try {
        return std::stoi(value);
    } catch (...) {
        throw http::BadRequestException(
            std::format("invalid value for header {}: {}", key, value));
    }
}

void CheckMethod(const std::unordered_set<http::Method>& supported,
                 http::Method method) {
    if (!supported.contains(method)) {
        throw http::NotFoundError();
    }
}

inference::Message FindMessageForRole(const std::string_view role,
                                      json::ArrayValue& msgs) {
    auto it =
        std::find_if(msgs.begin(), msgs.end(), [role](json::ValuePtr val) {
            return *val->as_object().at("role")->as_string() == role;
        });
    if (it == msgs.end()) {
        throw http::BadRequestException(std::format("role {} not found", role));
    }
    auto msg = (*it)->as_object();
    return inference::Message{
        .role = *msg.at("role")->as_string(),
        .content = *msg.at("content")->as_string(),
    };
}

inference::Request ExtractRequest(json::ValuePtr json_request) {
    auto msgs = json_request->as_object().at("messages")->as_array();
    inference::Message system = FindMessageForRole("system", msgs);
    inference::Message user = FindMessageForRole("user", msgs);
    return inference::Request{
        .system_message = system,
        .user_message = user,
    };
}

json::ValuePtr StubResponse() {
    return json::Parse(R"(
    {
        "id": "gabby-completion-123",
        "object": "chat.completion",
        "created": 1111111111,
        "model": "gabby-model",
        "system_fingerprint": "fp_1111111111",
        "choices": [
        ],
        "usage": {
            "prompt_tokens": 1,
            "completion_tokens": 1,
            "total_tokens": 1,
            "completion_tokens_details": {
                "reasoning_tokens": 1,
                "accepted_prediction_tokens": 0,
                "rejected_prediction_tokens": 0
            }
        }
    }
)");
}

json::ValuePtr MakeResponse(const inference::Message& answer) {
    auto response = StubResponse();
    auto choice = json::Value::Object({
        {"index", json::Value::Number(0)},
        {"logprobs", json::Value::Nil()},
        {"finish_reason", json::Value::String("stop")},
        {"message", json::Value::Object({
                        {"role", json::Value::String(answer.role)},
                        {"content", json::Value::String(answer.content)},
                    })},
    });
    response->as_object().at("choices")->as_array().push_back(choice);
    return response;
}

}  // namespace

InferenceService::InferenceService(Config config)
    : config_(config),
      server_(std::make_unique<http::HttpServer>(config_.server_config)),
      generator_(inference::Generator::LoadFromDirectory(config.model_dir)) {}

InferenceService::InferenceService(
    std::unique_ptr<http::HttpServer> server,
    std::unique_ptr<inference::Generator> generator)
    : server_(std::move(server)), generator_(std::move(generator)) {}

http::Handler InferenceService::HealthCheck() {
    return [](http::Request& req, http::ResponseWriter& resp) {
        resp.WriteStatus(http::StatusCode::OK);
    };
};

http::Handler InferenceService::ChatCompletions() {
    return [this](http::Request& req, http::ResponseWriter& resp) {
        // TODO: lift this into http::Router
        CheckMethod({http::Method::POST}, req.method);

        // TODO: read the full body when content-length is specified,
        // even if we don't use it, and lift this into http::Server
        int content_length = GetIntHeader(req, "Content-Length");

        auto json_req = json::Parse(req.stream, content_length);
        LOG(DEBUG) << "completion request: " << *json_req;

        inference::Request question = ExtractRequest(json_req);
        inference::Message answer = generator_->Generate(question);
        auto json_resp = MakeResponse(answer);
        LOG(DEBUG) << "completion response: " << *json_resp;

        resp.WriteStatus(http::StatusCode::OK);
        resp.WriteData(to_string(*json_resp));
    };
}

void InferenceService::Start() {
    //
    server_->Start(http::Router::builder()
                       .route("/healthz", HealthCheck())
                       .route("/v1/chat/completions", ChatCompletions())
                       .build());
}

void InferenceService::Wait() {
    //
    server_->Wait();
}

void InferenceService::Stop() {
    //
    server_->Stop();
}

}  // namespace gabby
