#include "server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "http/test_client.h"
#include "test/test.h"
#include "utils/logging.h"

namespace gabby {
namespace http {

constexpr ServerConfig kTestConfig{
    .port = 0,
    .read_timeout_millis = 5000,
    .write_timeout_millis = 5000,
    .worker_threads = 3,
};

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
        UnbufferedClientSocket sock(server.port());
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
    UnbufferedClientSocket sock(server.port());
    sock.Write("GET ");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto result = sock.ReadAll();

    // Assert
    // The sever should send a 408 before we finish the request.
    EXPECT_FALSE(*done);
    EXPECT_SUBSTR(result, "HTTP/1.1 408 Request Timeout");
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
    UnbufferedClientSocket sock(server.port());
    sock.Write("GET / HTTP/1.1\r\n\r\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto result = sock.ReadAll();

    // Assert
    // The server should send an OK, but then hang up before it
    // writes the full response.
    EXPECT_TRUE(*done);
    EXPECT_SUBSTR(result, "200 OK");
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
    EXPECT_SUBSTR(result, "HTTP/1.1 200 OK");
    EXPECT_SUBSTR(result, data);

    // TODO: implement EXPECT_THAT and matchers
    EXPECT_EQ(Method::GET, received.method);
    EXPECT_EQ("/foo", received.path);
    EXPECT_EQ(received.headers["a"], "b");
    EXPECT_EQ(received.headers["1"], "2");
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
        std::mutex mux;
        std::vector<std::string> results;
        for (int i = 0; i < num_clients; i++) {
            threads[i] =
                std::thread([&mux, &results, &ready, num_requests, port] {
                    ready.wait(false);
                    for (int j = 0; j < num_requests; j++) {
                        std::scoped_lock guard(mux);
                        results.push_back(Call(port, Method::GET, "/foo"));
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
        for (const std::string& result : results) {
            EXPECT_SUBSTR(result, "HTTP/1.1 200 OK");
        }
    }
}

}  // namespace http
}  // namespace gabby
