#include "server.h"

#include <sys/poll.h>
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

class Stream {
public:
    explicit Stream(int fd) {
        if ((f_ = fdopen(fd, "r+")) == nullptr) {
            throw std::system_error(errno, std::system_category());
        }
    }

    FILE* get() { return f_; }

    ~Stream() {
        fflush(f_);
        fclose(f_);
    }

private:
    FILE* f_;
};

HttpServer::HttpServer(const ServerConfig& config, Handler handler)
    : config_(config), sock_(config.port), handler_(handler) {
    if (::pipe(pipe_fds_) < 0) {
        throw std::system_error(errno, std::system_category());
    }
}

HttpServer::~HttpServer() {
    close(pipe_fds_[0]);
    close(pipe_fds_[1]);
}

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
    write(pipe_fds_[1], &done, 1);
}

void HttpServer::Listen() {
    sock_.Listen();
    LOG(INFO) << "listening at port " << sock_.port();
    struct pollfd fds[2];
    fds[0].fd = sock_.fd();
    fds[0].events = POLLIN;
    fds[1].fd = pipe_fds_[0];
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

    // set read and write timeouts on the socket
    struct timeval timeout;
    timeout.tv_usec = config_.read_timeout_millis * 1000;
    if (setsockopt(sock.fd(), SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    timeout.tv_usec = config_.write_timeout_millis * 1000;
    if (setsockopt(sock.fd(), SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        throw std::system_error(errno, std::system_category());
    }

    Stream stream(sock.fd());
    ResponseWriter resp(stream.get());
    Request req;
    try {
        req = Request::ParseFrom(stream.get());
    } catch (const RequestParsingException& e) {
        LOG(INFO) << sock.addr() << " - INVALID REQUEST 400 0";
        resp.WriteStatus(StatusCode::BadRequest);
        resp.Write(e.what());
        return;
    }

    handler_(req, resp);
    LOG(INFO) << sock.addr() << " - " << to_string(req.method) << " "
              << req.path << " HTTP/1.1 " << int(resp.status()) << " "
              << resp.bytes_written() << " " << req.user_agent;
}

}  // namespace http
}  // namespace gabby
