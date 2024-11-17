#include "api/openai.h"

namespace gabby {
namespace api {

http::Handler BuildOpenAIAPI() {
    return [](const http::Request& request, http::ResponseWriter& response) {
        response.SendStatus(http::StatusCode::OK);
    };
}

}  // namespace api
}  // namespace gabby
