#include "server.h"

#include <format>

#include "utils/logging.h"

namespace gabby {
namespace http {

HttpServer::HttpServer(const HttpServerConfig& config, Handler handler)
    : port_(config.port), handler_(handler) {}

void HttpServer::start() {
    LOG(INFO) << "starting http server at port " << port_;
}

void HttpServer::stop() { LOG(INFO) << "stopping http server"; }

}  // namespace http
}  // namespace gabby
