#include <iostream>
#include <string_view>

#include "service.h"
#include "utils/logging.h"

namespace gabby {

[[noreturn]] void Die(const std::string_view message) {
    std::cerr << message << std::endl;
    std::exit(1);
}

std::ostream& operator<<(std::ostream& os, const Config& config) {
    return os << "{ port: " << config.port
              << ", models_dir: " << config.models_dir
              << ", log_level: " << config.log_level << " }";
}

Config kDefaultConfig{.port = 8080, .log_level = LogLevel::OFF};

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

void Run(int argc, char* argv[]) {
    auto config = ParseArgs(argc, argv);
    LOG(INFO) << "server config: " << config;
    SetGlobalLogLevel(config.log_level);
    InferenceService service(config);
    service.start();
    // TODO: handle graceful shutdown
}

}  // namespace gabby

int main(int argc, char* argv[]) { gabby::Run(argc, argv); }
