#include "http/socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils/logging.h"

namespace gabby {
namespace http {

OwnedFd to_owned(int fd) {
    return OwnedFd(new int(fd), [](int* fdp) {
        if (fdp && *fdp >= 0) {
            close(*fdp);
            delete fdp;
        }
    });
}

ClientSocket::ClientSocket(OwnedFd fd, struct sockaddr_in addr)
    : fd_(std::move(fd)) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
    addr_ = std::string(ip);
    port_ = ntohs(addr.sin_port);
}

ServerSocket::ServerSocket(int port)
    : port_(port), fd_(to_owned(socket(AF_INET, SOCK_STREAM, 0))) {
    if (*fd_ < 0) throw SystemError(errno);
}

void ServerSocket::Listen() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    if (::bind(*fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    if (::listen(*fd_, SOMAXCONN) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    socklen_t len = sizeof(addr);
    getsockname(*fd_, (struct sockaddr*)&addr, &len);
    port_ = ntohs(addr.sin_port);
}

ClientSocket ServerSocket::Accept() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(*fd_, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        throw std::system_error(errno, std::system_category());
    }
    return ClientSocket(to_owned(client_fd), client_addr);
}

}  // namespace http
}  // namespace gabby
