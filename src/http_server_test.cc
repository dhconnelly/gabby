#include "http_server.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace gabby {
namespace http {

class MockResponseWriter : public ResponseWriter {
public:
    MOCK_METHOD(void, SendStatus, (StatusCode code), (override));
};

TEST(Router, HandleEmpty) {
    Request req{.addr = "1.2.3.4", .params = {}, .path = "/"};
    MockResponseWriter resp;
    EXPECT_CALL(resp, SendStatus(StatusCode::NotFound)).Times(1);
    auto handler = Router::builder().build();
    handler(req, resp);
}

}  // namespace http
}  // namespace gabby
