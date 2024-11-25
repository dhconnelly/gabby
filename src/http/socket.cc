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
    if (fd_ >= 0) close(fd_);
}

ClientSocket::ClientSocket(ClientSocket&& other) {
    fd_ = other.fd_;
    other.fd_ = -1;
}

ClientSocket& ClientSocket::operator=(ClientSocket&& other) {
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
}

ServerSocket::ServerSocket(int port) : port_(port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) SystemError(errno);
    fd_ = fd;
}

ServerSocket::~ServerSocket() {
    if (fd_ >= 0) close(fd_);
}

ServerSocket::ServerSocket(ServerSocket&& other) {
    fd_ = other.fd_;
    other.fd_ = -1;
}

ServerSocket& ServerSocket::operator=(ServerSocket&& other) {
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
}

void ServerSocket::Listen() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    if (::listen(fd_, SOMAXCONN) < 0) {
        throw std::system_error(errno, std::system_category());
    }
    socklen_t len = sizeof(addr);
    getsockname(fd_, (struct sockaddr*)&addr, &len);
    port_ = ntohs(addr.sin_port);
}

ClientSocket ServerSocket::Accept() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(fd_, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        throw std::system_error(errno, std::system_category());
    }
    return ClientSocket(client_fd, client_addr);
}

Pipe::Pipe() {
    if (::pipe(fds_) < 0) {
        throw std::system_error(errno, std::system_category());
    }
}

Pipe::~Pipe() {
    if (fds_[0] >= 0) {
        close(fds_[0]);
        close(fds_[1]);
    }
}

Pipe::Pipe(Pipe&& other) {
    fds_[0] = other.fds_[0];
    fds_[1] = other.fds_[1];
    other.fds_[0] = -1;
    other.fds_[1] = -1;
}

Pipe& Pipe::operator=(Pipe&& other) {
    fds_[0] = other.fds_[0];
    fds_[1] = other.fds_[1];
    other.fds_[0] = -1;
    other.fds_[1] = -1;
    return *this;
}

}  // namespace http
}  // namespace gabby
