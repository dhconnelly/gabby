#include "server.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <format>
#include <memory>
#include <stdexcept>
#include <thread>

#include "utils/logging.h"

using testing::AllOf;
using testing::Eq;
using testing::Field;
using testing::HasSubstr;
using testing::Not;
using testing::Pair;
using testing::UnorderedElementsAre;

namespace gabby {
namespace http {

constexpr int kNumRetries = 3;

constexpr ServerConfig kTestConfig{
    .port = 0,
    .read_timeout_millis = 5000,
    .write_timeout_millis = 5000,
    .worker_threads = 3,
};

// note: doesn't perform any buffering
class OutgoingSocket {
public:
    OutgoingSocket(OutgoingSocket&) = delete;
    OutgoingSocket(OutgoingSocket&&) = delete;
    OutgoingSocket& operator=(OutgoingSocket&) = delete;
    OutgoingSocket& operator=(OutgoingSocket&&) = delete;
    ~OutgoingSocket() { close(fd_); }

    explicit OutgoingSocket(int port) {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        while (connect(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            if (errno == ECONNABORTED) continue;
            else throw SystemError(errno);
        }
    }

    void Write(const std::string_view data) {
        if (data.empty()) return;
        for (int n = 0; n < data.size();) {
            int k = write(fd_, data.data() + n, data.size() - n);
            if (k < 0) {
                throw SystemError(errno);
            }
            n += k;
        }
    }

    std::string ReadAll() {
        std::string data;
        char buf[1024];
        int n;
        while ((n = read(fd_, buf, 1024)) > 0) {
            data.append(buf, n);
        }
        if (n < 0) throw SystemError(errno);
        return data;
    }

private:
    int fd_;
};

std::string Call(
    int port, Method method, const std::string_view path,
    const std::unordered_map<std::string, std::string>& headers = {}) {
    for (int attempt = 0; attempt < kNumRetries; attempt++) {
        try {
            OutgoingSocket sock(port);
            sock.Write(
                std::format("{} {} HTTP/1.1\r\n", to_string(method), path));
            for (const auto& [key, value] : headers) {
                sock.Write(std::format("{}: {}\r\n", key, value));
            }
            sock.Write("\r\n");
            return sock.ReadAll();
        } catch (SystemError& e) {
            if (e.error() == ECONNRESET) {
                continue;
            } else {
                throw e;
            }
        }
    }
    throw std::runtime_error("exceeded max retries");
}

class TestServer {
public:
    TestServer(ServerConfig config, Handler h) : server_(config, h) {
        server_.Start();
    }
    TestServer(Handler h) : TestServer(kTestConfig, h) {}

    int port() { return server_.port(); }

    ~TestServer() {
        server_.Stop();
        server_.Wait();
    }

private:
    HttpServer server_;
};

TEST(HttpServer, CallAndHangUp) {
    // Arrange
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    auto server = TestServer(kTestConfig,
                             [done](const Request& req, ResponseWriter& resp) {
                                 *done = true;
                                 resp.WriteStatus(StatusCode::OK);
                             });

    // Act
    // The server should handle this gracefully.
    for (int i = 0; i < 5; i++) {
        OutgoingSocket sock(server.port());
    }

    // Assert
    // The handler should never be invoked, and also the server should
    // not crash and should keep handling requests.
    EXPECT_FALSE(*done);
}

TEST(HttpServer, CallWithWriteDelay) {
    // Arrange
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    Request received;
    auto config = kTestConfig;
    config.read_timeout_millis = 1;
    auto server = TestServer(
        config, [&received, done](const Request& req, ResponseWriter& resp) {
            *done = true;
            received = req;
            resp.WriteStatus(StatusCode::OK);
        });

    // Act
    // Sleep before sending the full request line.
    OutgoingSocket sock(server.port());
    sock.Write("GET ");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto result = sock.ReadAll();

    // Assert
    // The sever should send a 408 before we finish the request.
    EXPECT_FALSE(*done);
    EXPECT_THAT(result, HasSubstr("HTTP/1.1 408 Request Timeout"));
}

TEST(HttpServer, CallWithReadDelay) {
    // Arrange
    // Send enough data from the server to fill up the socket buffer.
    std::string data(16 * 1024 * 1024, 'x');
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    Request received;
    auto config = kTestConfig;
    config.write_timeout_millis = 1;
    auto server = TestServer(
        config,
        [&received, &data, done](const Request& req, ResponseWriter& resp) {
            *done = true;
            received = req;
            resp.WriteStatus(StatusCode::OK);
            resp.WriteData(data);
        });

    // Act
    // Sleep so we can't ACK the full response.
    OutgoingSocket sock(server.port());
    sock.Write("GET / HTTP/1.1\r\n\r\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto result = sock.ReadAll();

    // Assert
    // The server should send an OK, but then hang up before it
    // writes the full response.
    EXPECT_TRUE(*done);
    EXPECT_THAT(result, HasSubstr("200 OK"));
    EXPECT_EQ(result.find(data), std::string::npos);
}

TEST(HttpServer, CallSuccessfully) {
    // Arrange
    std::string data(16 * 1024 * 1024, 'x');
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    Request received;
    auto server = TestServer(
        [&received, &data, done](const Request& req, ResponseWriter& resp) {
            *done = true;
            received = req;
            resp.WriteStatus(StatusCode::OK);
            resp.WriteData(data);
        });

    // Act
    auto result = Call(server.port(), Method::GET, "/foo",
                       {
                           {"a", "b"},
                           {"1", "2"},
                       });

    // Assert
    // The server should successfully read the request and successfully
    // send the full response.
    EXPECT_TRUE(*done);
    EXPECT_THAT(result, HasSubstr("HTTP/1.1 200 OK"));
    EXPECT_NE(result.find(data), std::string::npos);
    EXPECT_THAT(
        received,
        AllOf(Field(&Request::method, Method::GET),
              Field(&Request::path, "/foo"),
              Field(&Request::headers,
                    UnorderedElementsAre(Pair("a", "b"), Pair("1", "2")))));
}

TEST(HttpServer, CallConcurrently) {
    for (int num_workers = 1; num_workers <= 7; num_workers++) {
        // Arrange
        auto config = kTestConfig;
        config.worker_threads = num_workers;
        std::atomic<int> count = 0;
        auto server = TestServer(
            config, [&count](const Request& req, ResponseWriter& resp) {
                count++;
                resp.WriteStatus(StatusCode::OK);
            });
        int port = server.port();

        // Act
        std::atomic<bool> ready = false;
        int num_clients = 10;
        int num_requests = 10;
        std::vector<std::thread> threads(num_clients);
        for (int i = 0; i < num_clients; i++) {
            threads[i] = std::thread([&ready, num_requests, port] {
                ready.wait(false);
                for (int j = 0; j < num_requests; j++) {
                    auto result = Call(port, Method::GET, "/foo");
                    EXPECT_THAT(result, HasSubstr("HTTP/1.1 200 OK"));
                }
            });
        }
        ready = true;
        ready.notify_all();
        for (int i = 0; i < num_clients; i++) {
            threads[i].join();
        }

        // Assert
        EXPECT_EQ(count, num_clients * num_requests);
    }
}

}  // namespace http
}  // namespace gabby
