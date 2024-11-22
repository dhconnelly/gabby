#include "http/types.h"

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

void ResponseWriter::Write(std::string_view s) {
    if (s.size() == 0) return;
    if (fwrite(s.data(), 1, s.size(), stream_) == 0) {
        LOG(ERROR) << std::format("failed to write to socket: {}",
                                  strerror(errno));
        throw InternalError("failed to write data");
    }
}

void ResponseWriter::WriteStatus(StatusCode status) {
    if (sending_data_) {
        LOG(ERROR) << std::format("can't send status {}, already sending data",
                                  int(status));
        throw InternalError("failed to send http status");
    }
    status_ = status;
    Write(std::format("HTTP/1.1 {} {}\r\n", int(status), to_string(status)));
    WriteHeader("Connection", "close");
}

void ResponseWriter::WriteHeader(std::string key, std::string value) {
    if (sending_data_) {
        LOG(ERROR) << std::format("can't send header {}, already sending data",
                                  key);
        throw InternalError("failed to send http status");
    }
    headers_[key] = value;
    Write(std::format("{}: {}\r\n", key, value));
}

void ResponseWriter::WriteData(std::string_view data) {
    if (!sending_data_) {
        if (!status_.has_value()) {
            WriteStatus(StatusCode::OK);
        }
        Write("\r\n");
        sending_data_ = true;
    }
    if (fwrite(data.data(), 1, data.size(), stream_) < data.size()) {
        LOG(ERROR) << std::format("failed to write data: {}", strerror(errno));
        throw InternalError("failed to send data");
    }
}

}  // namespace http
}  // namespace gabby
