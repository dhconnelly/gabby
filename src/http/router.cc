#include "http/router.h"

#include "utils/logging.h"

namespace gabby {
namespace http {

void Router::handle(Request& req, ResponseWriter& resp) {
    LOG(DEBUG) << "handling path " << req.path;
    std::smatch m;
    for (auto& [pat, re, h] : routes_) {
        LOG(DEBUG) << "testing handler " << pat;
        if (std::regex_match(req.path, m, re)) {
            return h(req, resp);
        }
    }
    LOG(WARN) << "no handler for path " << req.path;
    resp.SendStatus(StatusCode::NotFound);
}

Router::Builder& Router::Builder::route(std::string pat, Handler handler) {
    routes_.push_back(
        {.pat = pat, .re = std::regex(pat), .handler = std::move(handler)});
    return *this;
}

Handler Router::Builder::build() {
    Router router(std::move(routes_));
    return [router = std::move(router)](Request& req,
                                        ResponseWriter& resp) mutable {
        router.handle(req, resp);
    };
}

}  // namespace http
}  // namespace gabby
