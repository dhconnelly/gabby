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
    if (method_str == "GET") {
        return Method::GET;
    } else if (method_str == "POST") {
        return Method::POST;
    } else {
        throw BadRequestException("invalid http method");
    }
}

std::string ParsePath(const std::string_view request_line) {
    int from = request_line.find(" ");
    int to = request_line.find(" ", from + 1);
    if (from == std::string_view::npos || to == std::string_view::npos) {
        throw BadRequestException("missing http path");
    }
    return std::string(request_line.begin() + from + 1,
                       request_line.begin() + to);
}

std::pair<std::string, std::string> ParseHeader(const std::string_view line) {
    int delim = line.find(": ");
    if (delim == std::string_view::npos) {
        throw BadRequestException("missing colon in http header");
    }
    std::string key(line.begin(), line.begin() + delim);
    std::string value(line.begin() + delim + 2, line.end());
    return {key, value};
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

    req->stream = stream;
}

}  // namespace

HttpServer::HttpServer(const ServerConfig& config, Handler handler)
    : config_(config),
      sock_(config.port),
      handler_(handler),
      running_(new std::atomic(false)),
      run_(new std::atomic(false)) {}

void HttpServer::Start() {
    *run_ = true;
    listener_thread_ = std::make_unique<std::thread>(&HttpServer::Listen, this);
    LOG(DEBUG) << "waiting for server to be ready";
    running_->wait(false);
    LOG(DEBUG) << "server running";
}

void HttpServer::Wait() { listener_thread_->join(); }

void HttpServer::Stop() {
    *run_ = false;
    char done = 1;
    write(pipe_.writefd(), &done, 1);
    running_->wait(true);
    LOG(DEBUG) << "server stopped running";
}

void HttpServer::Listen() {
    sock_.Listen();
    *running_ = true;
    running_->notify_one();
    LOG(INFO) << "http server listening at port " << sock_.port();

    struct pollfd fds[2];
    fds[0].fd = sock_.fd();
    fds[0].events = POLLIN;
    fds[1].fd = pipe_.readfd();
    fds[1].events = POLLIN;
    while (*run_) {
        int ret = poll(fds, 2, -1);
        assert(ret != 0);  // impossible: no timeout
        if (ret < 0) throw std::system_error(errno, std::system_category());
        if (fds[0].revents & POLLIN) {
            Handle(sock_.Accept());
        }
    }

    *running_ = false;
    running_->notify_one();
    LOG(INFO) << "http server stopped";
}

void HttpServer::Handle(ClientSocket&& sock) {
    // TODO: concurrency

    LOG(DEBUG) << "handling client " << sock.addr() << ":" << sock.port();

    // set read and write timeouts on the socket
    struct timeval timeout;
    timeout.tv_sec = config_.read_timeout_millis / 1000;
    timeout.tv_usec = (config_.read_timeout_millis % 1000) * 1000;
    if (setsockopt(sock.fd(), SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    timeout.tv_sec = config_.write_timeout_millis / 1000;
    timeout.tv_usec = (config_.write_timeout_millis % 1000) * 1000;
    if (setsockopt(sock.fd(), SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   sizeof(timeout)) < 0) {
        throw std::system_error(errno, std::system_category());
    }

    Stream stream(sock.fd());
    ResponseWriter resp(stream.get());
    Request req{.addr = sock.addr()};
    try {
        ParseRequest(&req, stream.get());
        handler_(req, resp);
        if (!resp.status().has_value()) {
            throw InternalError("no response sent");
        }
    } catch (const HttpException& e) {
        LOG(WARN) << e.what();
        LOG(INFO) << sock.addr() << " - " << to_string(e.status()) << " "
                  << int(e.status()) << " 0";
        resp.WriteStatus(e.status());
        resp.WriteData(e.what());
        resp.WriteData("\r\n");
        return;
    }

    std::string user_agent = req.headers["User-Agent"];
    LOG(INFO) << sock.addr() << " - " << to_string(req.method) << " "
              << req.path << " HTTP/1.1 " << int(*resp.status()) << " "
              << resp.bytes_written() << " " << user_agent;
}

}  // namespace http
}  // namespace gabby
