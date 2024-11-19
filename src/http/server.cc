#include "server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <format>
#include <stdexcept>
#include <thread>

#include "utils/logging.h"

namespace gabby {
namespace http {

HttpServer::ServerSocket::ServerSocket(int port) : port_(port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::system_error(errno, std::system_category());
    fd_ = fd;
}

HttpServer::ServerSocket::~ServerSocket() { close(fd_); }

void HttpServer::ServerSocket::listen() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    if (::listen(fd_, SOMAXCONN) < 0) {
        throw std::system_error(errno, std::system_category());
    }
}

HttpServer::HttpServer(const HttpServerConfig& config, Handler handler)
    : sock_(config.port), handler_(handler) {}

void HttpServer::start() {
    LOG(INFO) << "starting http server at port " << sock_.port();
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
    sock_.listen();
    LOG(INFO) << "listening at port " << sock_.port();
    while (running_) {
        LOG(DEBUG) << "waiting for connection...";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        LOG(DEBUG) << "got a connection or a timeout.";
    }
    LOG(DEBUG) << "server stopped, exiting listener";
}

}  // namespace http
}  // namespace gabby
