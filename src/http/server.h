#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <netinet/in.h>

#include <memory>
#include <thread>

#include "http/types.h"

namespace gabby {
namespace http {

class ClientSocket {
public:
    ClientSocket(int fd, struct sockaddr_in addr);
    ~ClientSocket();
    int port() { return port_; }
    std::string addr() { return addr_; }

private:
    int fd_;
    int port_;
    std::string addr_;
};

class ServerSocket {
public:
    explicit ServerSocket(int port);
    ~ServerSocket();
    int fd() { return fd_; }
    int port() { return port_; }
    void Listen();
    ClientSocket Accept();

private:
    int port_;
    int fd_;
};

struct HttpServerConfig {
    int port;
    int shutdown_timeout;
};

class HttpServer {
public:
    HttpServer(const HttpServerConfig& config, Handler handler);
    void Start();
    void Stop();

private:
    void Listen();
    void Handle(ClientSocket&& sock);

    ServerSocket sock_;
    Handler handler_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> listener_thread_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
