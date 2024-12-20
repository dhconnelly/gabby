#ifndef GABBY_HTTP_TYPES_H_
#define GABBY_HTTP_TYPES_H_

#include <functional>
#include <optional>
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
    RequestTimeout = 408,
    InternalServerError = 500,
};

std::ostream& operator<<(std::ostream& os, StatusCode status);

class HttpException : public std::exception {
public:
    explicit HttpException(std::string what) : what_(what) {}
    const char* what() const noexcept override { return what_.c_str(); }
    virtual StatusCode status() const = 0;

private:
    std::string what_;
};

class InternalError : public HttpException {
public:
    explicit InternalError(std::string what) : HttpException(what) {}
    StatusCode status() const override {
        return StatusCode::InternalServerError;
    }
};

class BadRequestException : public HttpException {
public:
    explicit BadRequestException(std::string what) : HttpException(what) {}
    StatusCode status() const override { return StatusCode::BadRequest; }
};

class TimeoutException : public HttpException {
public:
    explicit TimeoutException() : HttpException("request timeout") {}
    StatusCode status() const override { return StatusCode::RequestTimeout; }
};

class NotFoundError : public HttpException {
public:
    explicit NotFoundError() : HttpException("not found") {}
    StatusCode status() const override { return StatusCode::NotFound; }
};

enum class Method {
    GET,
    POST,
};

std::ostream& operator<<(std::ostream&, Method method);
std::string to_string(StatusCode code);
std::string to_string(Method method);

struct Request {
    std::string addr;
    Method method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    FILE* stream;
};

std::ostream& operator<<(std::ostream& os, const Request& req);

class ResponseWriter {
public:
    virtual ~ResponseWriter() {}

    virtual void Flush() = 0;

    // writes an http header with the specified status code. it is an
    // error to call this twice or after any other data has been sent.
    // TODO: return a different object here to enfroce this.
    virtual void WriteStatus(StatusCode code) = 0;

    // writes the specified http header. it is an error to call this
    // after any other data has been sent.
    // TODO: return a different object here to enfroce this.
    virtual void WriteHeader(std::string key, std::string value) = 0;

    // writes the specified data into the response. if a status has not
    // already been sent, this will write StatusCode::OK first.
    virtual void WriteData(std::string_view data) = 0;

    virtual std::optional<StatusCode> status() = 0;
};

using Handler = std::function<void(Request& request, ResponseWriter& response)>;

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_TYPES_H_
