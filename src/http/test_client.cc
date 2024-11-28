#include "http/test_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <format>
#include <string>
#include <string_view>

#include "http/types.h"
#include "json/parser.h"
#include "utils/logging.h"

namespace gabby {
namespace http {

constexpr int kNumRetries = 3;

UnbufferedClientSocket::~UnbufferedClientSocket() { close(fd_); }

UnbufferedClientSocket::UnbufferedClientSocket(int port) {
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    while (connect(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if (errno == ECONNABORTED) continue;
        else throw SystemError(errno);
    }
}

void UnbufferedClientSocket::Write(const std::string_view data) {
    if (data.empty()) return;
    for (int n = 0; n < data.size();) {
        int k = write(fd_, data.data() + n, data.size() - n);
        if (k < 0) {
            throw SystemError(errno);
        }
        n += k;
    }
}

std::string UnbufferedClientSocket::ReadAll() {
    std::string data;
    char buf[1024];
    int n;
    while ((n = read(fd_, buf, 1024)) > 0) {
        data.append(buf, n);
    }
    if (n < 0) throw SystemError(errno);
    return data;
}

void WriteHeader(UnbufferedClientSocket& sock, const std::string_view key,
                 const std::string_view value) {
    sock.Write(std::format("{}: {}\r\n", key, value));
}

std::string Call(int port, Method method, const std::string_view path,
                 const std::unordered_map<std::string, std::string>& headers,
                 const std::string_view data) {
    for (int attempt = 0; attempt < kNumRetries; attempt++) {
        try {
            UnbufferedClientSocket sock(port);

            LOG(DEBUG) << "writing request line";
            sock.Write(
                std::format("{} {} HTTP/1.1\r\n", to_string(method), path));

            LOG(DEBUG) << "writing headers";
            for (const auto& [key, value] : headers) {
                WriteHeader(sock, key, value);
                sock.Write(std::format("{}: {}\r\n", key, value));
            }
            if (!data.empty()) {
                WriteHeader(sock, "Content-Length",
                            std::to_string(data.size()));
                sock.Write(
                    std::format("{}: {}\r\n", "Content-Length", data.size()));
            }

            LOG(DEBUG) << "writing data";
            sock.Write("\r\n");
            if (!data.empty()) {
                sock.Write(data);
            }

            LOG(DEBUG) << "reading response";
            auto response = sock.ReadAll();

            LOG(DEBUG) << "read " << response.size() << " bytes response";
            return response;
        } catch (SystemError& e) {
            if (e.error() == ECONNRESET) {
                continue;
            } else {
                throw e;
            }
        }
    }
    throw std::runtime_error("exceeded max retries");
}

json::ValuePtr PostJson(int port, const std::string_view path,
                        const json::ValuePtr data) {
    auto result = Call(port, Method::POST, path, {}, json::to_string(*data));
    LOG(DEBUG) << "result: " << result;
    auto split = result.find("\r\n\r\n");
    LOG(DEBUG) << "split at " << split;
    if (split == std::string::npos) {
        throw std::invalid_argument("invalid http response");
    }
    LOG(DEBUG) << "extracting response part";
    auto rest = result.substr(split);
    LOG(DEBUG) << "parsing: " << rest;
    auto parsed = json::Parse(rest);
    LOG(DEBUG) << "parsed json: " << *parsed;
    return parsed;
}

}  // namespace http
}  // namespace gabby
