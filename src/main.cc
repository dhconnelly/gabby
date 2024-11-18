#include <iostream>
#include <string_view>

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

std::ostream& operator<<(std::ostream& os, const Config& config) {
    return os << "{ port: " << config.port
              << ", log_level: " << config.log_level << " }";
}

Config kDefaultConfig{.port = 8080, .log_level = LogLevel::OFF};

Config ParseArgs(int argc, char* argv[]) {
    Config config = kDefaultConfig;
    for (int i = 0; i < argc; i++) {
        std::string_view arg(argv[i]);
        if (arg == "--port") {
            if (i == argc - 1) Die("missing argument for --port");
            int port = atoi(argv[i]);
            if (port == 0) Die("invalid --port argument");
            config.port = port;
        } else if (arg == "--info") {
            config.log_level = LogLevel::INFO;
        } else if (arg == "--warn") {
            config.log_level = LogLevel::WARN;
        } else if (arg == "--debug") {
            config.log_level = LogLevel::DEBUG;
        }
    }
    return config;
}

struct Context {};

http::Handler MakeRouter() {
    std::shared_ptr<Context> ctx(new Context);
    return http::Router::builder()
        .route("/healthz",
               [ctx](const http::Request& request,
                     http::ResponseWriter& response) {
                   LOG(INFO) << "handling healthz";
                   response.SendStatus(http::StatusCode::OK);
               })
        .route("/",
               [ctx](const http::Request& request,
                     http::ResponseWriter& response) {
                   LOG(INFO) << "handling /";
                   response.SendStatus(http::StatusCode::OK);
               })
        .build();
}

void Run(int argc, char* argv[]) {
    auto config = gabby::ParseArgs(argc, argv);
    gabby::SetGlobalLogLevel(config.log_level);
    LOG(INFO) << "server config: " << config;
    gabby::http::HttpServer server(gabby::MakeRouter());
    server.run();
}

}  // namespace gabby

int main(int argc, char* argv[]) { gabby::Run(argc, argv); }
