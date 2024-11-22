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

#include "utils/logging.h"

using testing::HasSubstr;

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

HttpServer TestServer(Handler h) {
    SetGlobalLogLevel(LogLevel::DEBUG);
    HttpServer server(kTestConfig, h);
    return std::move(server);
}

TEST(HttpServer, Handle) {
    std::shared_ptr<std::atomic<bool>> done(new std::atomic(false));
    auto server = TestServer([done](const Request& req, ResponseWriter& resp) {
        *done = true;
        resp.WriteStatus(StatusCode::OK);
    });
    LOG(DEBUG) << "created test server, starting it";
    server.Start();
    LOG(DEBUG) << "calling test server";
    auto resp = Call(server.port(), Method::GET, "/", {});
    EXPECT_THAT(resp, HasSubstr("HTTP/1.1 200 OK"));
    LOG(DEBUG) << "got response: " << resp;
    server.Stop();
    server.Wait();
}

}  // namespace http
}  // namespace gabby
