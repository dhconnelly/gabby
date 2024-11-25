#ifndef GABBY_HTTP_SOCKET_H_
#define GABBY_HTTP_SOCKET_H_

#include <netinet/in.h>

#include <memory>
#include <string>

namespace gabby {
namespace http {

using OwnedFd = std::unique_ptr<int, void (*)(int*)>;

OwnedFd to_owned(int fd);

// resource handle that wraps a socket for a client connection
class ClientSocket {
public:
    int port() const { return port_; }
    std::string addr() const { return addr_; }
    int fd() { return *fd_; }

private:
    ClientSocket(OwnedFd fd, struct sockaddr_in addr);
    friend class ServerSocket;

    OwnedFd fd_;
    int port_;
    std::string addr_;
};

// resource handle that binds and listens to a specified port
class ServerSocket {
public:
    explicit ServerSocket(int port);
    int fd() const { return *fd_; }
    int port() const { return port_; }

    // starts listening at the specified port
    void Listen();

    // accepts a client connection
    ClientSocket Accept();

private:
    int port_;
    OwnedFd fd_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SOCKET_H_
