#include "http/types.h"

#include <iostream>

namespace gabby {
namespace http {

std::string to_string(StatusCode code) {
    switch (code) {
        case StatusCode::OK: return "OK";
        case StatusCode::NotFound: return "NotFound";
        case StatusCode::BadRequest: return "BadRequest";
        case StatusCode::InternalServerError: return "InternalServerError";
    }
}

std::string to_string(Method method) {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
    }
}

}  // namespace http
}  // namespace gabby
