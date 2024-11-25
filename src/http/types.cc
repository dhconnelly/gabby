#include "http/types.h"

#include <cstring>
#include <format>
#include <iostream>

#include "unistd.h"
#include "utils/logging.h"

namespace gabby {
namespace http {

std::string to_string(StatusCode code) {
    switch (code) {
        case StatusCode::OK: return "OK";
        case StatusCode::NotFound: return "Not Found";
        case StatusCode::BadRequest: return "Bad Request";
        case StatusCode::RequestTimeout: return "Request Timeout";
        case StatusCode::InternalServerError: return "Internal Server Error";
    }
}

std::string to_string(Method method) {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
    }
}

std::ostream& operator<<(std::ostream& os, StatusCode status) {
    return os << to_string(status);
}

std::ostream& operator<<(std::ostream& os, Method method) {
    return os << to_string(method);
}

std::ostream& operator<<(std::ostream& os, const Request& req) {
    os << "{ ";
    os << std::format("addr: {}", req.addr);
    os << std::format(", method: {}", to_string(req.method));
    os << std::format(", path: {}", req.path);
    os << ", headers: { ";
    for (const auto& [k, v] : req.headers) {
        os << std::format("{}: {}, ", k, v);
    }
    os << " }";
    os << " }";
    return os;
}

}  // namespace http
}  // namespace gabby
