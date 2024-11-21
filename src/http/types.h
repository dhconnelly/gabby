#ifndef GABBY_HTTP_TYPES_H_
#define GABBY_HTTP_TYPES_H_

#include <functional>
#include <ostream>
#include <string>
#include <unordered_map>

#include "json/json.h"

namespace gabby {
namespace http {

enum class StatusCode : int {
    OK = 200,
    BadRequest = 400,
    NotFound = 404,
    InternalServerError = 500,
};

enum class Method {
    GET,
    POST,
};

std::string to_string(StatusCode code);
std::string to_string(Method method);

class RequestParsingException : public std::exception {
public:
    explicit RequestParsingException(std::string what) : what_(what) {}
    const char* what() const noexcept override { return what_.c_str(); }

private:
    std::string what_;
};

struct Request {
    std::string addr;
    Method method;
    std::string user_agent;
    std::string path;
    std::unordered_map<std::string, std::string> params;
    FILE* stream;

    static Request ParseFrom(FILE* stream);
};

class ResponseWriter {
public:
    explicit ResponseWriter(FILE* stream) : stream_(stream) {}
    virtual ~ResponseWriter() { fflush(stream_); }

    // writes an http header with the specified status code. it is an
    // error to call this twice or after any other data has been sent.
    // TODO: return a different object here to enfroce this.
    virtual void WriteStatus(StatusCode code);

    // writes the specified data into the response. if a status has not
    // already been sent, this will write StatusCode::OK first.
    virtual void Write(std::string_view data);

    StatusCode status() { return status_; }
    int bytes_written() { return bytes_written_; }

private:
    FILE* stream_;
    StatusCode status_;
    int bytes_written_ = 0;
};

using Handler = std::function<void(Request& request, ResponseWriter& response)>;

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_TYPES_H_
