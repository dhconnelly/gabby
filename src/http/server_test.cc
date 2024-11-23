#include "server.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <atomic>
#include <cstdio>
#include <format>
#include <memory>
#include <stdexcept>

using testing::AllOf;
using testing::Eq;
using testing::Field;
using testing::HasSubstr;
using testing::Pair;
using testing::UnorderedElementsAre;

namespace gabby {
namespace http {

constexpr ServerConfig kTestConfig{
    .port = 0,
    .read_timeout_millis = 5000,
    .write_timeout_millis = 5000,
    .idle_timeout_millis = 5000,
};

class OutgoingSocket {
public:
    explicit OutgoingSocket(int port) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::system_error(errno, std::system_category());
        }
        if ((stream_ = fdopen(fd, "r+")) == nullptr) {
            throw std::system_error(errno, std::system_category());
        }
    }

    void Write(const std::string_view data) {
        if (data.empty()) return;
        if (fwrite(data.data(), 1, data.size(), stream_) != data.size()) {
            throw std::system_error(errno, std::system_category());
        }
    }

    std::string ReadAll() {
        std::string data;
        char buf[1024];
        while (true) {
            int n = fread(buf, 1, 1024, stream_);
            if (::ferror(stream_)) {
                throw std::system_error(errno, std::system_category());
            }
            if (n == 0) {
                break;
            }
            data.append(buf, n);
        }
        return data;
    }

    ~OutgoingSocket() { fclose(stream_); }

private:
    FILE* stream_;
};

std::string Call(int port, Method method, const std::string_view path,
                 const std::unordered_map<std::string, std::string>& headers) {
    OutgoingSocket sock(port);
    sock.Write(std::format("{} {} HTTP/1.1\r\n", to_string(method), path));
    for (const auto& [key, value] : headers) {
        sock.Write(std::format("{}: {}\r\n", key, value));
    }
    sock.Write("\r\n");
    return sock.ReadAll();
}

class TestServer {
public:
    TestServer(Handler h) : server_(kTestConfig, h) { server_.Start(); }

    int port() { return server_.port(); }

    ~TestServer() {
        server_.Stop();
        server_.Wait();
    }

private:
    HttpServer server_;
};

TEST(HttpServer, ParseRequest) {
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    Request received;
    auto server =
        TestServer([&received, done](const Request& req, ResponseWriter& resp) {
            *done = true;
            received = req;
            resp.WriteStatus(StatusCode::OK);
        });
    auto result = Call(server.port(), Method::GET, "/foo", {});
    EXPECT_THAT(result, HasSubstr("HTTP/1.1 200 OK"));
    EXPECT_THAT(received, AllOf(Field(&Request::method, Method::GET),
                                Field(&Request::path, "/foo")));
}

TEST(HttpServer, ParseHeaders) {
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    Request received;
    auto server =
        TestServer([&received, done](const Request& req, ResponseWriter& resp) {
            *done = true;
            received = req;
            resp.WriteStatus(StatusCode::OK);
        });
    auto result = Call(server.port(), Method::GET, "/foo",
                       {
                           {"a", "b"},
                           {"1", "2"},
                       });
    EXPECT_THAT(result, HasSubstr("HTTP/1.1 200 OK"));
    EXPECT_THAT(
        received,
        AllOf(Field(&Request::method, Method::GET),
              Field(&Request::path, "/foo"),
              Field(&Request::headers,
                    UnorderedElementsAre(Pair("a", "b"), Pair("1", "2")))));
}

}  // namespace http
}  // namespace gabby
