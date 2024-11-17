#include <iostream>
#include <string_view>

#include "api.h"
#include "http_server.h"

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

Config kDefaultConfig = Config{
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

int main(int argc, char* argv[]) {
    auto config = ParseArgs(argc, argv);
    std::cout << config << std::endl;

    gabby::HttpServer server({.port = config.port});
    server.run();

    return 0;
}
