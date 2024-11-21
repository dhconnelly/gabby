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

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SOCKET_H_
