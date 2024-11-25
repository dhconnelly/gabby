#include <csignal>
#include <cstring>
#include <format>
#include <iostream>
#include <string_view>
#include <thread>

#include "service.h"
#include "utils/logging.h"

namespace gabby {

[[noreturn]] void Die(const std::string_view message) {
    std::cerr << message << std::endl;
    std::exit(1);
}

std::ostream& operator<<(std::ostream& os, const Config& config) {
    return os << "{ server_config: " << config.server_config  //
              << ", models_dir: " << config.models_dir        //
              << ", log_level: " << config.log_level          //
              << " }";
}

Config kDefaultConfig{
    .log_level = LogLevel::OFF,
    .models_dir = "",
    .server_config =
        http::ServerConfig{
            .port = 8080,
            .read_timeout_millis = 5'000,
            .write_timeout_millis = 10'000,
            .worker_threads = std::thread::hardware_concurrency() - 1,
        },
};

bool ParseIntFlag(int argc, char* argv[], std::string_view flag, int* argi,
                  int* val) {
    if (argv[*argi] != flag) return false;
    if (*argi + 1 == argc) Die(std::format("missing argument for {}", flag));
    *val = std::stoi(argv[*argi + 1]);
    (*argi)++;
    return true;
}

Config ParseArgs(int argc, char* argv[]) {
    Config config = kDefaultConfig;
    for (int i = 1; i < argc; i++) {
        if (ParseIntFlag(argc, argv, "--port", &i,
                         &config.server_config.port)) {
        } else if (ParseIntFlag(argc, argv, "--read_timeout_millis", &i,
                                &config.server_config.read_timeout_millis)) {
        } else if (ParseIntFlag(argc, argv, "--write_timeout_millis", &i,
                                &config.server_config.write_timeout_millis)) {
        } else if (strcmp(argv[i], "--info") == 0) {
            config.log_level = LogLevel::INFO;
        } else if (strcmp(argv[i], "--warn") == 0) {
            config.log_level = LogLevel::WARN;
        } else if (strcmp(argv[i], "--debug") == 0) {
            config.log_level = LogLevel::DEBUG;
        } else {
            Die(std::format("invalid argument: {}", argv[i]));
        }
    }
    return config;
}

static InferenceService* service = nullptr;

constexpr const char* signame(int signal) {
    switch (signal) {
        case SIGINT: return "SIGINT";
        case SIGTERM: return "SIGTERM";
        default: return "";
    }
}

void shutdown(int signal) {
    LOG(INFO) << "received " << signame(signal);
    service->Stop();
}

void Run(int argc, char* argv[]) {
    auto config = ParseArgs(argc, argv);
    SetGlobalLogLevel(config.log_level);
    LOG(INFO) << "server config: " << config;
    service = new InferenceService(config);
    std::signal(SIGINT, shutdown);
    std::signal(SIGTERM, shutdown);
    service->Start();
    service->Wait();
    LOG(INFO) << "exiting";
}

}  // namespace gabby

int main(int argc, char* argv[]) { gabby::Run(argc, argv); }
