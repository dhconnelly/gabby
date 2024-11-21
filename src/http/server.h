#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <netinet/in.h>

#include <memory>
#include <optional>
#include <thread>

#include "http/types.h"

namespace gabby {
namespace http {

class ClientSocket {
public:
    ClientSocket(int fd, struct sockaddr_in addr);
    ~ClientSocket();
    int port() const { return port_; }
    std::string addr() const { return addr_; }
    int fd() { return fd_; }

private:
    int fd_;
    int port_;
    std::string addr_;
};

class ServerSocket {
public:
    explicit ServerSocket(int port);
    ~ServerSocket();
    int fd() const { return fd_; }
    int port() const { return port_; }
    void Listen();
    ClientSocket Accept();

private:
    int port_;
    int fd_;
};

class Pipe {
public:
    Pipe();
    ~Pipe();
    int readfd() { return fds_[0]; }
    int writefd() { return fds_[1]; }

private:
    int fds_[2];
};

struct ServerConfig {
    int port;
    int read_timeout_millis;
    int write_timeout_millis;
    int idle_timeout_millis;
};

class HttpServer {
public:
    HttpServer(const ServerConfig& config, Handler handler);
    void Start();
    void Stop();

private:
    void Listen();
    void Handle(ClientSocket&& sock);

    ServerSocket sock_;
    Handler handler_;
    Pipe exit_pipe_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> listener_thread_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
