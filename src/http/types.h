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

std::ostream& operator<<(std::ostream& os, StatusCode code);

struct Request {
    std::string addr;
    std::string path;
    std::unordered_map<std::string, std::string> params;

    json::Value json() { return json::Value{}; }
};

class ResponseWriter {
public:
    virtual ~ResponseWriter() = default;

    // writes an http header with the specified status code. it is an
    // error to call this twice or after any other data has been sent.
    // TODO: return a different object here to enfroce this.
    virtual void SendStatus(StatusCode code) = 0;
    // writes the specified data into the response. if a status has not
    // already been sent, this will write StatusCode::OK first.
    virtual void Send(std::string_view data) = 0;
};

using Handler = std::function<void(Request& request, ResponseWriter& response)>;

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_TYPES_H_
