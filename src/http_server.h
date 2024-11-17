#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

namespace gabby {

struct HttpServerConfig {
    int port;
};

class HttpServer {
public:
    HttpServer(const HttpServerConfig& config) : port_(config.port) {}
    void run();

private:
    int port_;
};

}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
