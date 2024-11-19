#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include "http/types.h"

namespace gabby {
namespace http {

struct HttpServerConfig {
    int port;
};

class HttpServer {
public:
    HttpServer(const HttpServerConfig& config, Handler handler);
    void start();
    void stop();

private:
    int port_;
    Handler handler_;
};

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_SERVER_H_
