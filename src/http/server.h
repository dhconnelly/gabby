#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <iostream>
#include <memory>
#include <optional>
#include <thread>

#include "http/types.h"

namespace gabby {
namespace http {

struct ServerConfig {
    int port;
    int read_timeout_millis;
    int write_timeout_millis;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

using OwnedFd = std::unique_ptr<int, void (*)(int*)>;

class HttpServer {
public:
    HttpServer(const ServerConfig& config, Handler handler);
    int port() { return port_; }

    void Start();
    void Wait();
    void Stop();

private:
    struct Client {
        OwnedFd fd;
        int port;
        std::string addr;
    };

    void Listen();
    void Accept();
    void Handle(Client&& sock);

    ServerConfig config_;
    int port_;
    OwnedFd sock_;
    Handler handler_;
    // used by poll() for graceful shutdown. [read, write]
    std::array<OwnedFd, 2> pipe_;
    std::unique_ptr<std::atomic<bool>> run_;
    std::unique_ptr<std::atomic<bool>> running_;
    std::unique_ptr<std::thread> listener_thread_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
