#include <iostream>
#include <string_view>

#include "api/openai.h"
#include "http/router.h"
#include "http/server.h"
#include "http/types.h"
#include "log.h"

namespace gabby {

[[noreturn]] void Die(const std::string_view message) {
    std::cerr << message << std::endl;
    std::exit(1);
}

struct Config {
    int port;
    LogLevel log_level;
};

Config kDefaultConfig{.port = 8080, .log_level = LogLevel::OFF};

std::ostream& operator<<(std::ostream& os, const Config& config) {
    return os << "{ port: " << config.port
              << ", log_level: " << config.log_level << " }";
}

Config ParseArgs(int argc, char* argv[]) {
    Config config = kDefaultConfig;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0) {
            if (i == argc - 1) Die("missing argument for --port");
            int port = atoi(argv[i + 1]);
            if (port == 0) Die("invalid --port argument");
            config.port = port;
        } else if (strcmp(argv[i], "--info") == 0) {
            config.log_level = LogLevel::INFO;
        } else if (strcmp(argv[i], "--warn") == 0) {
            config.log_level = LogLevel::WARN;
        } else if (strcmp(argv[i], "--debug") == 0) {
            config.log_level = LogLevel::DEBUG;
        }
    }
    return config;
}

http::Handler HealthCheck() {
    return [](const http::Request& req, http::ResponseWriter& resp) {
        resp.SendStatus(http::StatusCode::OK);
    };
}

void Run(int argc, char* argv[]) {
    auto config = gabby::ParseArgs(argc, argv);
    SetGlobalLogLevel(config.log_level);
    LOG(INFO) << "server config: " << config;

    http::HttpServer server(http::HttpServerConfig{.port = config.port},
                            http::Router::builder()
                                .route("/healthz", HealthCheck())
                                .route("/v1/.*", api::BuildOpenAIAPI())
                                .build());
    server.run();
}

}  // namespace gabby

int main(int argc, char* argv[]) { gabby::Run(argc, argv); }
