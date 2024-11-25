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
    int port = 0;
    int read_timeout_millis = 5000;
    int write_timeout_millis = 5000;
    // must be at least 1
    unsigned int worker_threads = 1;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

using OwnedFd = std::unique_ptr<int, void (*)(int*)>;

class HttpServer {
public:
    HttpServer(const ServerConfig& config, Handler handler);
    int port() { return port_; }
    int total_threads() { return config_.worker_threads + 1; }

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
    void WorkerAccept(int id);
    void Accept();
    void Handle(Client&& sock);

    ServerConfig config_;
    int port_;
    OwnedFd sock_;
    Handler handler_;

    // used by the poll() loop for graceful shutdown.
    std::array<OwnedFd, 2> pipe_;  // [read, write]
    std::atomic<bool> run_;        // set to false to shut down
    std::atomic<bool> running_;    // set to indicate we can accept clients
    std::vector<std::thread> threads_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
