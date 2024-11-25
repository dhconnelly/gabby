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

namespace {

constexpr int kMaxLineLen = 256;

std::string_view ReadLine(char buf[], FILE* stream) {
    ::fgets(buf, kMaxLineLen, stream);
    if (::ferror(stream) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        throw TimeoutException{};
    }
    if (::ferror(stream)) {
        throw BadRequestException("failed to read from stream");
    }
    if (::feof(stream)) {
        throw BadRequestException("unexpected eof");
    }
    int read = strnlen(buf, kMaxLineLen);
    assert(read > 0);
    if (buf[read - 1] != '\n') {
        throw BadRequestException("header line too long");
    }
    if (read >= 2 && buf[read - 2] == '\r') {
        return std::string_view(buf, buf + read - 2);
    }
    throw BadRequestException("invalid line ending");
}

Method ParseMethod(const std::string_view request_line) {
    int to = request_line.find(" ");
    if (to == std::string_view::npos) {
        throw BadRequestException("missing http method");
    }
    std::string_view method_str(request_line.begin(),
                                request_line.begin() + to);
    Method method;
    if (method_str == "GET") {
        method = Method::GET;
    } else if (method_str == "POST") {
        method = Method::POST;
    } else {
        throw BadRequestException("invalid http method");
    }
    LOG(DEBUG) << "parsed method: " << method;
    return method;
}

std::string ParsePath(const std::string_view request_line) {
    int from = request_line.find(" ");
    int to = request_line.find(" ", from + 1);
    if (from == std::string_view::npos || to == std::string_view::npos) {
        throw BadRequestException("missing http path");
    }
    std::string path(request_line.begin() + from + 1,
                     request_line.begin() + to);
    LOG(DEBUG) << "parsed path: " << path;
    return path;
}

std::pair<std::string, std::string> ParseHeader(const std::string_view line) {
    int delim = line.find(": ");
    if (delim == std::string_view::npos) {
        throw BadRequestException("missing colon in http header");
    }
    std::string k(line.begin(), line.begin() + delim);
    std::string v(line.begin() + delim + 2, line.end());
    LOG(DEBUG) << "parsed header: " << std::format("[{}: {}]", k, v);
    return {k, v};
}

void ParseRequest(Request* req, FILE* stream) {
    char buf[kMaxLineLen];
    std::string_view line = ReadLine(buf, stream);
    if (line.empty()) throw BadRequestException("missing request line");
    req->method = ParseMethod(line);
    req->path = ParsePath(line);
    while (!(line = ReadLine(buf, stream)).empty()) {
        req->headers.insert(ParseHeader(line));
    }
    LOG(DEBUG) << "parsed request: " << *req;
    req->stream = stream;
}

}  // namespace

std::ostream& operator<<(std::ostream& os, const ServerConfig& config) {
    return os << "{ port: " << config.port
              << ", read_timeout_millis: " << config.read_timeout_millis
              << ", write_timeout_millis: " << config.write_timeout_millis
              << " }";
}

std::array<OwnedFd, 2> MakePipe() {
    int fds[2];
    if (::pipe(fds) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    return {to_owned(fds[0]), to_owned(fds[1])};
}

HttpServer::HttpServer(const ServerConfig& config, Handler handler)
    : config_(config),
      sock_(config.port),
      handler_(handler),
      pipe_(MakePipe()),
      running_(new std::atomic(false)),
      run_(new std::atomic(false)) {}

void HttpServer::Start() {
    *run_ = true;
    listener_thread_ = std::make_unique<std::thread>(&HttpServer::Listen, this);
    LOG(DEBUG) << "waiting for server to be ready...";
    running_->wait(false);
    LOG(DEBUG) << "server ready.";
}

void HttpServer::Wait() {
    LOG(DEBUG) << "waiting on server thread to exit...";
    listener_thread_->join();
    LOG(DEBUG) << "server thread exited.";
}

void HttpServer::Stop() {
    LOG(DEBUG) << "stopping server...";
    *run_ = false;
    char done = 1;
    write(*pipe_[1], &done, 1);
    running_->wait(true);
    LOG(DEBUG) << "stopped server";
}

void HttpServer::Accept() {
    try {
        Handle(sock_.Accept());
    } catch (std::system_error& e) {
        if (e.code().value() == ECONNABORTED) {
            LOG(WARN) << e.what();
        } else {
            throw e;
        }
    }
}

void HttpServer::Listen() {
    sock_.Listen();
    LOG(INFO) << "http server listening at port " << sock_.port();

    *running_ = true;
    running_->notify_one();

    LOG(DEBUG) << "http server loop started";
    struct pollfd fds[2];
    fds[0].fd = sock_.fd();
    fds[0].events = POLLIN;
    fds[1].fd = *pipe_[0];
    fds[1].events = POLLIN;
    while (*run_) {
        int ret = poll(fds, 2, -1);
        assert(ret != 0);  // impossible: no timeout
        if (ret < 0) throw SystemError(errno);
        if (fds[0].revents & POLLIN) Accept();
    }
    LOG(DEBUG) << "http server loop finished";

    *running_ = false;
    running_->notify_one();
}

namespace {

void SetTimeout(int fd, int millis, int mask) {
    struct timeval timeout;
    timeout.tv_sec = millis / 1000;
    timeout.tv_usec = (millis % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, mask, &timeout, sizeof(timeout)) < 0) {
        throw SystemError(errno);
    }
}

void MustSend(ResponseWriter& resp, StatusCode status) noexcept {
    if (resp.status().has_value()) {
        LOG(ERROR) << "can't send " << status << ", already sent "
                   << *resp.status();
        return;
    }
    try {
        resp.WriteStatus(status);
        resp.Flush();
    } catch (std::exception& e) {
        LOG(WARN) << "failed to send " << status << ": " << e.what();
        return;
    }
}

}  // namespace

void HttpServer::Handle(ClientSocket&& sock) {
    // TODO: concurrency
    LOG(DEBUG) << "handling client " << sock.addr() << ":" << sock.port();

    SetTimeout(sock.fd(), config_.read_timeout_millis, SO_RCVTIMEO);
    SetTimeout(sock.fd(), config_.write_timeout_millis, SO_SNDTIMEO);

    FILE* stream;
    if ((stream = fdopen(sock.fd(), "r+")) == nullptr) {
        throw SystemError(errno);
    }

    ResponseWriter resp(stream);
    Request req{.addr = sock.addr()};
    try {
        ParseRequest(&req, stream);
        handler_(req, resp);
        resp.Flush();
        std::string user_agent = req.headers["User-Agent"];
        LOG(INFO) << sock.addr() << " - " << to_string(req.method) << " "
                  << req.path << " HTTP/1.1 " << int(*resp.status()) << " "
                  << resp.bytes_written() << " " << user_agent;
    } catch (const HttpException& e) {
        LOG(ERROR) << e.what();
        MustSend(resp, e.status());
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        MustSend(resp, StatusCode::InternalServerError);
    }

    LOG(DEBUG) << "done handling client " << sock.addr() << ":" << sock.port();
}

}  // namespace http
}  // namespace gabby
