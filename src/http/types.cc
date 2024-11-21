#include "http/types.h"

#include <iostream>

#include "unistd.h"

namespace gabby {
namespace http {

std::string to_string(StatusCode code) {
    switch (code) {
        case StatusCode::OK: return "OK";
        case StatusCode::NotFound: return "NotFound";
        case StatusCode::BadRequest: return "BadRequest";
        case StatusCode::InternalServerError: return "InternalServerError";
    }
}

std::string to_string(Method method) {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
    }
}

constexpr int kMaxHeaderLineLen = 256;

bool ReadLine(int fd, char buf[], int maxlen) {
    int read = 0;
    while (true) {
        int n = ::read(fd, buf, maxlen - read);
        if (n < 0) throw std::system_error(errno, std::system_category());
    }
}

/* static */
Request Request::ParseFrom(FILE *stream) {
    throw RequestParsingException("unimplemented");
}

void ResponseWriter::WriteStatus(StatusCode status) {
    status_ = status;
    // TODO
}

void ResponseWriter::Write(std::string_view data) {
    // TODO
}

}  // namespace http
}  // namespace gabby
