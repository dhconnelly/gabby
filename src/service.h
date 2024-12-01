#ifndef GABBY_SERVICE_H_
#define GABBY_SERVICE_H_

#include "http/server.h"
#include "http/types.h"
#include "utils/logging.h"

namespace gabby {

struct Config {
    LogLevel log_level;
    http::ServerConfig server_config;
    std::string model_dir;
};

class InferenceService {
public:
    explicit InferenceService(Config config);
    void Start();
    void Wait();
    void Stop();

    int port() const { return server_.port(); }

private:
    http::Handler HealthCheck();
    http::Handler ChatCompletions();

    Config config_;
    http::HttpServer server_;
};

}  // namespace gabby

#endif  // GABBY_SERVICE_H_
