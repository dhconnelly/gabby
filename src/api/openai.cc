#include "api/openai.h"

#include "http/router.h"
#include "log.h"

namespace gabby {
namespace api {

struct Context {};

http::Handler BuildOpenAIAPI() {
    std::shared_ptr<Context> ctx(new Context);
    return http::Router::builder()
        .route("/healthz",
               [ctx](const http::Request& request,
                     http::ResponseWriter& response) {
                   LOG(INFO) << "handling healthz";
                   response.SendStatus(http::StatusCode::OK);
               })
        .route("/",
               [ctx](const http::Request& request,
                     http::ResponseWriter& response) {
                   LOG(INFO) << "handling /";
                   response.SendStatus(http::StatusCode::OK);
               })
        .build();
}

}  // namespace api
}  // namespace gabby
