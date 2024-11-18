#include "server.h"

#include <format>

#include "log.h"

namespace gabby {
namespace http {

void HttpServer::run() { LOG(INFO) << "starting web server at port " << port_; }

}  // namespace http
}  // namespace gabby
