#include "service.h"

#include "http/router.h"
#include "model/loader.h"
#include "utils/logging.h"

namespace gabby {

InferenceService::InferenceService(Config config)
    : config_(config),
      models_(model::LoadModels(config.models_dir)),
      server_(config_.server_config,
              http::Router::builder()
                  .route("/healthz", HealthCheck())
                  .route("/v1/chat/completion", ChatCompletion())
                  .build()) {}

http::Handler InferenceService::HealthCheck() {
    return [](http::Request& req, http::ResponseWriter& resp) {
        resp.WriteStatus(http::StatusCode::OK);
    };
};

http::Handler InferenceService::ChatCompletion() {
    return [](http::Request& req, http::ResponseWriter& resp) {
        resp.WriteStatus(http::StatusCode::OK);
    };
}

void InferenceService::Start() {
    //
    server_.Start();
}

void InferenceService::Stop() {
    //
    server_.Stop();
}

}  // namespace gabby
