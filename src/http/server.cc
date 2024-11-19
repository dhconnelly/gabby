#include "server.h"

#include <chrono>
#include <format>
#include <thread>

#include "utils/logging.h"

namespace gabby {
namespace http {

HttpServer::HttpServer(const HttpServerConfig& config, Handler handler)
    : port_(config.port), handler_(handler) {}

void HttpServer::start() {
    LOG(INFO) << "starting http server at port " << port_;
    running_ = true;
    listener_thread_ = std::make_unique<std::thread>(&HttpServer::listen, this);
    listener_thread_->join();
    LOG(INFO) << "http server stopped";
}

void HttpServer::stop() {
    LOG(INFO) << "stopping http server...";
    running_ = false;
}

void HttpServer::listen() {
    while (running_) {
        LOG(DEBUG) << "waiting for connection...";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        LOG(DEBUG) << "got a connection or a timeout.";
    }
    LOG(DEBUG) << "server stopped, exiting listener";
}

}  // namespace http
}  // namespace gabby
