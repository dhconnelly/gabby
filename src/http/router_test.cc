#include "router.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace gabby {
namespace http {

Handler SimpleHandler(StatusCode code) {
    return [code](const Request& request, ResponseWriter& resp) {
        resp.SendStatus(code);
    };
}

Handler OKHandler = SimpleHandler(StatusCode::OK);
Handler NotFoundHandler = SimpleHandler(StatusCode::NotFound);
Handler ErrorHandler = SimpleHandler(StatusCode::InternalServerError);

class MockResponseWriter : public ResponseWriter {
public:
    MOCK_METHOD(void, SendStatus, (StatusCode code), (override));
};

TEST(Router, NoRoutesReturnsNotFound) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/"};
    MockResponseWriter resp;
    EXPECT_CALL(resp, SendStatus(StatusCode::NotFound)).Times(1);
    auto handler = Router::builder().build();
    handler(req, resp);
}

TEST(Router, NoMatchesReturnsNotFound) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/"};
    MockResponseWriter resp;
    EXPECT_CALL(resp, SendStatus(StatusCode::NotFound)).Times(1);
    auto handler = Router::builder()
                       .route("/foo", ErrorHandler)
                       .route("/bar", ErrorHandler)
                       .build();
    handler(req, resp);
}

TEST(Router, FirstMatchOnlyHandled) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/foo/bar/baz"};
    MockResponseWriter resp;
    EXPECT_CALL(resp, SendStatus(StatusCode::OK)).Times(1);
    auto handler = Router::builder()
                       .route("/foo/baz", ErrorHandler)
                       .route("/foo/bar/b.*", OKHandler)
                       .route("/foo.*", ErrorHandler)
                       .build();
    handler(req, resp);
}

}  // namespace http
}  // namespace gabby
