#ifndef GABBY_SERVICE_H_
#define GABBY_SERVICE_H_

#include "http/server.h"
#include "http/types.h"
#include "model/model.h"
#include "utils/logging.h"

namespace gabby {

struct Config {
    LogLevel log_level;
    std::string models_dir;
    http::ServerConfig server_config;
};

class InferenceService {
public:
    explicit InferenceService(Config config);
    void Start();
    void Stop();

private:
    http::Handler HealthCheck();
    http::Handler ChatCompletion();

    Config config_;
    http::HttpServer server_;
    std::unordered_map<std::string, model::Model> models_;
};

}  // namespace gabby

#endif  // GABBY_SERVICE_H_
