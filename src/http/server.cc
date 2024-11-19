#include "server.h"

#include <arpa/inet.h>
#include <sys/poll.h>
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

ClientSocket::ClientSocket(int fd, struct sockaddr_in addr) : fd_(fd) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
    addr_ = std::string(ip);
    port_ = ntohs(addr.sin_port);
}

ClientSocket::~ClientSocket() {
    LOG(DEBUG) << "closing client socket " << addr_ << ":" << port_;
    close(fd_);
}

ServerSocket::ServerSocket(int port) : port_(port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::system_error(errno, std::system_category());
    fd_ = fd;
}

ServerSocket::~ServerSocket() {
    LOG(DEBUG) << "closing server socket :" << port_;
    close(fd_);
}

void ServerSocket::Listen() {
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

ClientSocket ServerSocket::Accept() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(fd_, (struct sockaddr*)&client_addr, &addr_len);
    return ClientSocket(client_fd, client_addr);
}

HttpServer::HttpServer(const HttpServerConfig& config, Handler handler)
    : sock_(config.port), handler_(handler) {}

void HttpServer::Start() {
    LOG(INFO) << "starting http server at port " << sock_.port();
    running_ = true;
    listener_thread_ = std::make_unique<std::thread>(&HttpServer::Listen, this);
    listener_thread_->join();
    LOG(INFO) << "http server stopped";
}

void HttpServer::Stop() {
    LOG(INFO) << "stopping http server...";
    running_ = false;
}

void HttpServer::Listen() {
    sock_.Listen();
    LOG(INFO) << "listening at port " << sock_.port();
    struct pollfd fds[1];
    fds[0].fd = sock_.fd();
    fds[0].events = POLLIN;
    while (running_) {
        int ret = poll(fds, 1, 100 /*ms*/);
        if (ret < 0) throw std::system_error(errno, std::system_category());
        if (ret > 0) Handle(sock_.Accept());
    }
    LOG(DEBUG) << "server stopped, exiting listener";
}

void HttpServer::Handle(ClientSocket&& sock) {
    LOG(DEBUG) << "handling client " << sock.addr() << ":" << sock.port();
    auto req = ParseRequest(sock);
    ResponseWriter resp(sock.fd());
    handler_(req, resp);
    LOG(INFO) << to_string(req.method) << " " << req.path << " HTTP/1.1 "
              << int(resp.status()) << " " << resp.bytes_written()
              << " \"UNKNOWN\"";
}

Request HttpServer::ParseRequest(ClientSocket& sock) {
    // TODO
    Request req{.addr = sock.addr()};
    return req;
}

}  // namespace http
}  // namespace gabby
