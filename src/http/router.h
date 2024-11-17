#ifndef GABBY_HTTP_ROUTER_H_
#define GABBY_HTTP_ROUTER_H_

#include <regex>
#include <vector>

#include "http/types.h"

namespace gabby {
namespace http {

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

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_ROUTER_H_
