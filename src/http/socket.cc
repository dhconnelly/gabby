#include "http/socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils/logging.h"

namespace gabby {
namespace http {

ClientSocket::ClientSocket(int fd, struct sockaddr_in addr) : fd_(fd) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
    addr_ = std::string(ip);
    port_ = ntohs(addr.sin_port);
}

ClientSocket::~ClientSocket() {
    LOG(DEBUG) << "closing client socket " << addr_ << ":" << port_;
    close(fd_);
}

ServerSocket::ServerSocket(int port) : port_(port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::system_error(errno, std::system_category());
    fd_ = fd;
}

ServerSocket::~ServerSocket() {
    LOG(DEBUG) << "closing server socket :" << port_;
    close(fd_);
}

void ServerSocket::Listen() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    if (::listen(fd_, SOMAXCONN) < 0) {
        throw std::system_error(errno, std::system_category());
    }
}

ClientSocket ServerSocket::Accept() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(fd_, (struct sockaddr*)&client_addr, &addr_len);
    return ClientSocket(client_fd, client_addr);
}

}  // namespace http
}  // namespace gabby
