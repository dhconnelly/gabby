#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <string_view>
#include <thread>

#include "inference/config.h"
#include "service.h"
#include "utils/logging.h"

namespace gabby {
namespace {

namespace fs = std::filesystem;

[[noreturn]] void Die(const std::string_view message) {
    std::cerr << message << std::endl;
    std::exit(1);
}

std::ostream& operator<<(std::ostream& os, const Config& config) {
    return os << "{ server_config: " << config.server_config  //
              << ", log_level: " << config.log_level          //
              << ", model_dir: " << config.model_dir          //
              << " }";
}

Config DefaultConfig() {
    return Config{
        .log_level = LogLevel::OFF,
        .server_config =
            http::ServerConfig{
                .port = 8080,
                .read_timeout_millis = 5'000,
                .write_timeout_millis = 10'000,
                .worker_threads = std::thread::hardware_concurrency() - 1,
            },
        .model_dir = "",
    };
}

template <typename IntTypeT>
bool ParseIntFlag(int argc, char* argv[], std::string_view flag, int* argi,
                  IntTypeT* val) {
    if (argv[*argi] != flag) return false;
    if (*argi + 1 == argc) Die(std::format("missing argument for {}", flag));
    *val = std::stoi(argv[*argi + 1]);
    (*argi)++;
    return true;
}

bool ParseStrFlag(int argc, char* argv[], std::string_view flag, int* argi,
                  std::string* val) {
    // TODO: unify this with ParseIntFlag
    if (argv[*argi] != flag) return false;
    if (*argi + 1 == argc) Die(std::format("missing argument for {}", flag));
    *val = argv[*argi + 1];
    (*argi)++;
    return true;
}

Config ParseConfig(int argc, char* argv[]) {
    Config config = DefaultConfig();
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
        } else if (ParseIntFlag(argc, argv, "--workers", &i,
                                &config.server_config.worker_threads)) {
        } else if (ParseStrFlag(argc, argv, "--model-dir", &i,
                                &config.model_dir)) {
        } else {
            Die(std::format("invalid argument: {}", argv[i]));
        }
    }
    if (config.model_dir.empty()) {
        config.model_dir = inference::FindDefaultModelDir();
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
    auto config = ParseConfig(argc, argv);
    SetGlobalLogLevel(config.log_level);
    LOG(INFO) << "server config: " << config;
    service = new InferenceService(config);
    std::signal(SIGINT, shutdown);
    std::signal(SIGTERM, shutdown);
    service->Start();
    service->Wait();
    LOG(INFO) << "exiting";
}

}  // namespace
}  // namespace gabby

int main(int argc, char* argv[]) { gabby::Run(argc, argv); }
