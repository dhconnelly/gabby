#ifndef GABBY_HTTP_SERVER_H_
#define GABBY_HTTP_SERVER_H_

#include <memory>
#include <ostream>
#include <regex>
#include <vector>

namespace gabby {
namespace http {

enum class StatusCode : int {
    OK = 200,
    NotFound = 404,
    InternalServerError = 500,
};

std::ostream& operator<<(std::ostream& os, StatusCode code);

struct HttpServerConfig {
    int port;
};

constexpr HttpServerConfig kDefaultHttpServerConfig{
    .port = 8080,
};

struct Request {
    std::string addr;
    std::string path;
    std::unordered_map<std::string, std::string> params;
};

class ResponseWriter {
public:
    virtual void SendStatus(StatusCode code);
};

using Handler =
    std::function<void(const Request& request, ResponseWriter& response)>;

class Router {
public:
    struct Route {
        std::string pat;
        std::regex re;
        Handler handler;
    };

    class Builder {
    public:
        Builder& route(std::string pat, Handler handler);
        Handler build();

    private:
        std::vector<Route> routes_;
    };

    static Builder builder() { return Builder(); }
    void handle(const Request& request, ResponseWriter& response);

private:
    std::vector<Route> routes_;
    Router(std::vector<Route>&& routes) : routes_(routes) {}
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
