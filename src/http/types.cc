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
        os << std::format("{}: {}", k, v);
    }
    os << " }";
    os << " }";
    return os;
}

void ResponseWriter::Write(std::string_view s) {
    LOG(DEBUG) << "sending " << s.size() << " bytes in response";
    if (s.size() == 0) return;
    if (fwrite(s.data(), 1, s.size(), stream_) == 0) {
        if (ferror(stream_) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            throw TimeoutException{};
        }
        throw InternalError("failed to write data");
    }
    bytes_written_ += s.size();
}

void ResponseWriter::Flush() {
    LOG(DEBUG) << "flushing response to client";
    fflush(stream_);
}

void ResponseWriter::WriteStatus(StatusCode status) {
    if (sending_data_) {
        LOG(ERROR) << std::format("can't send status {}, already sending data",
                                  int(status));
        throw InternalError("failed to send http status");
    }
    Write(std::format("HTTP/1.1 {} {}\r\n", int(status), to_string(status)));
    WriteHeader("Connection", "close");
    status_ = status;
}

void ResponseWriter::WriteHeader(std::string key, std::string value) {
    if (sending_data_) {
        LOG(ERROR) << std::format("can't send header {}, already sending data",
                                  key);
        throw InternalError("failed to send http status");
    }
    Write(std::format("{}: {}\r\n", key, value));
    headers_[key] = value;
}

void ResponseWriter::WriteData(std::string_view data) {
    if (!sending_data_) {
        if (!status_.has_value()) {
            WriteStatus(StatusCode::OK);
        }
        Write("\r\n");
        sending_data_ = true;
    }
    Write(data);
}

}  // namespace http
}  // namespace gabby
