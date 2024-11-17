#include "server.h"

#include <format>

#include "log.h"

namespace gabby {
namespace http {

void HttpServer::run() { log::info("starting http server at port {}", port_); }

}  // namespace http
}  // namespace gabby
