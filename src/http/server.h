#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <iostream>
#include <memory>
#include <optional>
#include <thread>

#include "http/socket.h"
#include "http/types.h"

namespace gabby {
namespace http {

struct ServerConfig {
    int port;
    int read_timeout_millis;
    int write_timeout_millis;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);

class HttpServer {
public:
    HttpServer(const ServerConfig& config, Handler handler);
    void Start();
    void Wait();
    void Stop();
    int port() { return sock_.port(); }

private:
    void Listen();
    void Accept();
    void Handle(ClientSocket&& sock);

    ServerConfig config_;
    ServerSocket sock_;
    Handler handler_;
    Pipe pipe_;
    std::unique_ptr<std::atomic<bool>> run_;
    std::unique_ptr<std::atomic<bool>> running_;
    std::unique_ptr<std::thread> listener_thread_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
