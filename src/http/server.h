#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <thread>

#include "http/thread_pool.h"
#include "http/types.h"
#include "utils/pointers.h"

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

class HttpServer {
public:
    explicit HttpServer(const ServerConfig& config);
    ~HttpServer();

    int port() const { return port_; }
    int total_threads() const { return config_.worker_threads + 1; }

    // |handler| must be thread-safe
    void Start(Handler handler);
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
    std::unique_ptr<ThreadPool> pool_;

    // we run a poll() loop in a thread and use a pipe to notify it
    // about graceful shutdown
    std::thread listener_;
    std::array<OwnedFd, 2> pipe_;  // [read, write]
    std::atomic<bool> run_;        // set to false to shut down
    std::atomic<bool> running_;    // set to indicate we can accept clients
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
