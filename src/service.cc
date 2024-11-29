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

InferenceService::InferenceService(Config config)
    : config_(config),
      server_(config_.server_config,
              http::Router::builder()
                  .route("/healthz", HealthCheck())
                  .route("/v1/chat/completions", ChatCompletions())
                  .build()) {}

http::Handler InferenceService::HealthCheck() {
    return [](http::Request& req, http::ResponseWriter& resp) {
        resp.WriteStatus(http::StatusCode::OK);
    };
};

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

json::ValuePtr StubResponse() {
    return json::Parse(R"(
    {
        "id": "gabby-completion-123",
        "object": "chat.completion",
        "created": 1111111111,
        "model": "gabby-model",
        "system_fingerprint": "fp_1111111111",
        "choices": [{
            "index": 0,
            "message": {
                "role": "assistant",
                "content": "\n\nhey this is gabby"
            },
            "logprobs": null,
            "finish_reason": "stop"
        }],
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

json::ValuePtr ParseJson(FILE* stream, int size) {
    try {
        return json::Parse(stream, size);
    } catch (json::JSONError& e) {
        throw http::BadRequestException(std::format("bad json: {}", e.what()));
    }
}

void CheckMethod(const std::unordered_set<http::Method>& supported,
                 http::Method method) {
    if (!supported.contains(method)) {
        throw http::NotFoundError();
    }
}

}  // namespace

http::Handler InferenceService::ChatCompletions() {
    return [](http::Request& req, http::ResponseWriter& resp) {
        // TODO: lift this into http::Router
        CheckMethod({http::Method::POST}, req.method);

        // TODO: read the full body when content-length is specified,
        // even if we don't use it, and lift this into http::Server
        int content_length = GetIntHeader(req, "Content-Length");

        auto json_req = ParseJson(req.stream, content_length);
        LOG(DEBUG) << "completion request: " << *json_req;

        auto json_resp = StubResponse();
        LOG(DEBUG) << "completion response: " << *json_resp;

        resp.WriteStatus(http::StatusCode::OK);
        resp.WriteData(to_string(*json_resp));
    };
}

void InferenceService::Start() {
    //
    server_.Start();
}

void InferenceService::Wait() {
    //
    server_.Wait();
}

void InferenceService::Stop() {
    //
    server_.Stop();
}

}  // namespace gabby
