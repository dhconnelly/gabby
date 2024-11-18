#include "http/types.h"

#include <iostream>

namespace gabby {
namespace http {
std::ostream& operator<<(std::ostream& os, StatusCode code) {
    return os << static_cast<int>(code);
}

}  // namespace http
}  // namespace gabby
