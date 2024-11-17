#ifndef GABBY_API_OPENAI_H_
#define GABBY_API_OPENAI_H_

#include "http/types.h"

namespace gabby {
namespace api {

http::Handler BuildOpenAIAPI();

}  // namespace api
}  // namespace gabby

#endif  // GABBY_API_OPENAI_H_
