#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include "http/types.h"

namespace gabby {
namespace http {

struct HttpServerConfig {
    int port;
};

constexpr HttpServerConfig kDefaultHttpServerConfig{
    .port = 8080,
};

class HttpServer {
public:
    HttpServer(const HttpServerConfig& config, Handler handler)
        : port_(config.port) {}
    HttpServer(Handler handler)
        : HttpServer(kDefaultHttpServerConfig, handler) {}

    void run();

private:
    int port_;
    Handler handler_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
