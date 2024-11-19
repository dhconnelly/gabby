#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <memory>
#include <thread>

#include "http/types.h"

namespace gabby {
namespace http {

struct HttpServerConfig {
    int port;
};

class HttpServer {
public:
    HttpServer(const HttpServerConfig& config, Handler handler);
    void start();
    void stop();

private:
    void listen();

    class ServerSocket {
    public:
        ServerSocket(int port);
        void listen();
        int fd() { return fd_; }
        int port() { return port_; }
        ~ServerSocket();

    private:
        int port_;
        int fd_;
    };

    ServerSocket sock_;
    Handler handler_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> listener_thread_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
