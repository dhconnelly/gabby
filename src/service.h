#ifndef GABBY_SERVICE_H_
#define GABBY_SERVICE_H_

#include <memory>

#include "http/server.h"
#include "http/types.h"
#include "inference/generator.h"
#include "utils/logging.h"

namespace gabby {

struct Config {
    LogLevel log_level;
    http::ServerConfig server_config;
    std::string model_dir;
};

class InferenceService {
public:
    // TODO: make this static
    explicit InferenceService(Config config);

    InferenceService(std::unique_ptr<http::HttpServer> server,
                     std::unique_ptr<inference::Generator> generator);

    void Start();
    void Wait();
    void Stop();

    int port() const { return server_->port(); }

private:
    http::Handler HealthCheck();
    http::Handler ChatCompletions();

    Config config_;
    std::unique_ptr<http::HttpServer> server_;
    std::unique_ptr<inference::Generator> generator_;
};

}  // namespace gabby

#endif  // GABBY_SERVICE_H_
