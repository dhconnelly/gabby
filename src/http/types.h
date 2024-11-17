#ifndef GABBY_HTTP_TYPES_H_
#define GABBY_HTTP_TYPES_H_

#include <functional>
#include <ostream>
#include <string>
#include <unordered_map>

namespace gabby {
namespace http {

enum class StatusCode : int {
    OK = 200,
    NotFound = 404,
    InternalServerError = 500,
};

std::ostream& operator<<(std::ostream& os, StatusCode code);

struct Request {
    std::string addr;
    std::string path;
    std::unordered_map<std::string, std::string> params;
};

class ResponseWriter {
public:
    virtual void SendStatus(StatusCode code);
};

using Handler =
    std::function<void(const Request& request, ResponseWriter& response)>;

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_TYPES_H_
