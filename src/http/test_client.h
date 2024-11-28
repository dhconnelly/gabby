#ifndef GABBY_HTTP_CLIENT_H_
#define GABBY_HTTP_CLIENT_H_

#include <string>
#include <string_view>
#include <unordered_map>

#include "http/types.h"
#include "json/json.h"

namespace gabby {
namespace http {

class UnbufferedClientSocket {
public:
    UnbufferedClientSocket(UnbufferedClientSocket&) = delete;
    UnbufferedClientSocket(UnbufferedClientSocket&&) = delete;
    UnbufferedClientSocket& operator=(UnbufferedClientSocket&) = delete;
    UnbufferedClientSocket& operator=(UnbufferedClientSocket&&) = delete;
    ~UnbufferedClientSocket();

    explicit UnbufferedClientSocket(int port);
    void Write(const std::string_view data);
    std::string ReadAll();

private:
    int fd_;
};

std::string Call(
    int port, Method method, const std::string_view path,
    const std::unordered_map<std::string, std::string>& headers = {},
    const std::string_view data = "");

json::ValuePtr PostJson(int port, const std::string_view path,
                        const json::ValuePtr json);

}  // namespace http
}  // namespace gabby

#endif  // GABBY_HTTP_CLIENT_H_
