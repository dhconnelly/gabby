#include "service.h"

#include <cstdio>
#include <string>

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

static json::ValuePtr kStubResponse = json::Parse(R"(
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

}  // namespace

http::Handler InferenceService::ChatCompletions() {
    return [](http::Request& req, http::ResponseWriter& resp) {
        if (req.method != http::Method::POST) {
            throw http::NotFoundError();
        }

        auto content_length_it = req.headers.find("Content-Length");
        if (content_length_it == req.headers.end()) {
            throw http::BadRequestException("missing Content-Length");
        }
        int content_length;
        try {
            content_length = std::stoi(content_length_it->second);
        } catch (...) {
            throw http::BadRequestException("bad Content-Length");
        }

        LOG(DEBUG) << "reading " << content_length << " bytes as json...";
        json::ValuePtr request;
        try {
            request = json::Parse(req.stream, content_length);
        } catch (json::JSONError& e) {
            throw http::BadRequestException("bad json");
        }
        LOG(DEBUG) << "read json request: " << *request;

        resp.WriteStatus(http::StatusCode::OK);
        resp.WriteData(to_string(*kStubResponse));
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
