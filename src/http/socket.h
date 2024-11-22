#ifndef GABBY_HTTP_SOCKET_H_
#define GABBY_HTTP_SOCKET_H_

#include <netinet/in.h>

#include <string>

namespace gabby {
namespace http {

class ClientSocket {
public:
    ClientSocket(int fd, struct sockaddr_in addr);
    ~ClientSocket();
    ClientSocket(ClientSocket&&);
    ClientSocket& operator=(ClientSocket&&);
    ClientSocket(ClientSocket&) = delete;
    ClientSocket& operator=(ClientSocket&) = delete;

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
    ServerSocket(ServerSocket&&);
    ServerSocket& operator=(ServerSocket&&);
    ServerSocket(ServerSocket&) = delete;
    ServerSocket& operator=(ServerSocket&) = delete;

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
    Pipe(Pipe&&);
    Pipe& operator=(Pipe&&);
    Pipe(Pipe&) = delete;
    Pipe& operator=(Pipe&) = delete;
    int readfd() { return fds_[0]; }
    int writefd() { return fds_[1]; }

private:
    int fds_[2];
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SOCKET_H_
