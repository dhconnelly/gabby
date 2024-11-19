#ifndef GABBY_SERVICE_H_
#define GABBY_SERVICE_H_

#include "http/server.h"
#include "http/types.h"
#include "model/model.h"
#include "utils/logging.h"

namespace gabby {

struct Config {
    LogLevel log_level;
    int port;
    std::string models_dir;
};

class InferenceService {
public:
    InferenceService(Config config);
    void start();
    void stop();

private:
    http::Handler HealthCheck();
    http::Handler ChatCompletion();

    Config config_;
    http::HttpServer server_;
    std::unordered_map<std::string, model::Model> models_;
};

}  // namespace gabby

#endif  // GABBY_SERVICE_H_
