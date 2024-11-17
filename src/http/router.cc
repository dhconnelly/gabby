#include "http/router.h"

#include "log.h"

namespace gabby {
namespace http {

std::ostream& operator<<(std::ostream& os, StatusCode code) {
    return os << static_cast<int>(code);
}

void ResponseWriter::SendStatus(StatusCode code) {
    log::debug("sending status code {}", static_cast<int>(code));
}

void Router::handle(const Request& request, ResponseWriter& response) {
    log::debug("handling path {}", request.path);
    std::smatch m;
    for (auto& [pat, re, h] : routes_) {
        log::debug("testing handler {}", pat);
        if (std::regex_match(request.path, m, re)) {
            return h(request, response);
        }
    }
    log::warn("no handler for path {}", request.path);
    response.SendStatus(StatusCode::NotFound);
}

Router::Builder& Router::Builder::route(std::string pat, Handler handler) {
    routes_.push_back(
        {.pat = pat, .re = std::regex(pat), .handler = std::move(handler)});
    return *this;
}

Handler Router::Builder::build() {
    Router router(std::move(routes_));
    return [router = std::move(router)](const Request& request,
                                        ResponseWriter& response) mutable {
        router.handle(request, response);
    };
}

}  // namespace http
}  // namespace gabby
