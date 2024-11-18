#include "router.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace gabby {
namespace http {

Handler SimpleHandler(StatusCode code, std::string data = "") {
    return [code, data](const Request& request, ResponseWriter& resp) {
        resp.SendStatus(code);
        resp.Send(std::string_view(data));
    };
}

Handler NotFoundHandler = SimpleHandler(StatusCode::NotFound);
Handler ErrorHandler = SimpleHandler(StatusCode::InternalServerError);

struct SimpleResponseWriter : public http::ResponseWriter {
    void SendStatus(http::StatusCode code) override { this->code = code; }
    void Send(std::string_view data) override { this->data = data; }

    http::StatusCode code;
    std::string data;
};

TEST(Router, NoRoutesReturnsNotFound) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/"};
    SimpleResponseWriter resp;
    auto handler = Router::builder().build();
    handler(req, resp);
    EXPECT_EQ(StatusCode::NotFound, resp.code);
}

TEST(Router, NoMatchesReturnsNotFound) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/"};
    SimpleResponseWriter resp;
    auto handler = Router::builder()
                       .route("/foo", ErrorHandler)
                       .route("/bar", ErrorHandler)
                       .build();
    handler(req, resp);
    EXPECT_EQ(StatusCode::NotFound, resp.code);
}

TEST(Router, FirstMatchOnlyHandled) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/foo/bar/baz"};
    SimpleResponseWriter resp;
    auto handler = Router::builder()
                       .route("/foo/baz", ErrorHandler)
                       .route("/foo/bar/b.*",
                              SimpleHandler(http::StatusCode::OK, "success"))
                       .route("/foo.*", ErrorHandler)
                       .build();
    handler(req, resp);
    EXPECT_EQ(StatusCode::OK, resp.code);
    EXPECT_EQ("success", resp.data);
}

}  // namespace http
}  // namespace gabby
