#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

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
    int idle_timeout_millis;
};

class HttpServer {
public:
    HttpServer(const ServerConfig& config, Handler handler);
    ~HttpServer();
    void Start();
    void Stop();

private:
    void Listen();
    void Handle(ClientSocket&& sock);

    ServerConfig config_;
    ServerSocket sock_;
    Handler handler_;
    int pipe_fds_[2];
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> listener_thread_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
