#include "server.h"

#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <format>
#include <optional>
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

Pipe::Pipe() {
    if (::pipe(fds_) < 0) {
        throw std::system_error(errno, std::system_category());
    }
}

Pipe::~Pipe() {
    close(fds_[0]);
    close(fds_[1]);
}

HttpServer::HttpServer(const ServerConfig& config, Handler handler)
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
    char done = 1;
    write(exit_pipe_.writefd(), &done, 1);
}

void HttpServer::Listen() {
    sock_.Listen();
    LOG(INFO) << "listening at port " << sock_.port();
    struct pollfd fds[2];
    fds[0].fd = sock_.fd();
    fds[0].events = POLLIN;
    fds[1].fd = exit_pipe_.readfd();
    fds[1].events = POLLIN;
    while (running_) {
        int ret = poll(fds, 2, -1);
        assert(ret != 0);  // impossible: no timeout
        if (ret < 0) throw std::system_error(errno, std::system_category());
        if (fds[0].revents & POLLIN) {
            Handle(sock_.Accept());
        } else {
            assert(fds[1].revents & POLLIN);
            break;
        }
    }
    LOG(DEBUG) << "server stopped, exiting listener";
}

void HttpServer::Handle(ClientSocket&& sock) {
    // TODO: concurrency

    LOG(DEBUG) << "handling client " << sock.addr() << ":" << sock.port();
    ResponseWriter resp(sock.fd());
    Request req;
    try {
        req = Request::ParseFrom(sock.fd());
    } catch (const RequestParsingException& e) {
        LOG(INFO) << sock.addr() << " - INVALID REQUEST 400 0";
        resp.WriteStatus(StatusCode::BadRequest);
        resp.Write(e.what());
        return;
    }
    LOG(INFO) << sock.addr() << " - " << to_string(req.method) << " "
              << req.path << " HTTP/1.1 " << int(resp.status()) << " "
              << resp.bytes_written() << " " << req.user_agent;
    handler_(req, resp);
}

}  // namespace http
}  // namespace gabby
