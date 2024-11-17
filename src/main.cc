#include <iostream>
#include <string_view>

#include "http_server.h"
#include "log.h"

namespace gabby {

[[noreturn]] void Die(const std::string_view message) {
    std::cerr << message << std::endl;
    std::exit(1);
}

struct Config {
    int port;
};

std::ostream& operator<<(std::ostream& os, const Config& config) {
    return os << "{ " << "port: " << config.port << " }";
}

Config kDefaultConfig{
    .port = 8080,
};

Config ParseArgs(int argc, char* argv[]) {
    Config config = kDefaultConfig;
    for (int i = 0; i < argc; i++) {
        if (argv[i] == std::string_view("--port")) {
            if (i == argc - 1) Die("missing argument for --port");
            int port = atoi(argv[i]);
            if (port == 0) Die("invalid --port argument");
            config.port = port;
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
                   log::info("handling /healthz");
                   response.SendStatus(http::StatusCode::OK);
               })
        .route("/",
               [ctx](const http::Request& request,
                     http::ResponseWriter& response) {
                   log::info("handling /");
                   response.SendStatus(http::StatusCode::OK);
               })
        .build();
}

}  // namespace gabby

int main(int argc, char* argv[]) {
    auto config = gabby::ParseArgs(argc, argv);
    std::cout << config << std::endl;

    gabby::http::HttpServer server(gabby::MakeRouter());
    server.run();

    return 0;
}
