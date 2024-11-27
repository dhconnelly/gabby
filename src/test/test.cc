#include "test/test.h"

#include <format>
#include <iostream>

namespace gabby {

std::vector<TestCase*>* kTestCases = nullptr;

bool equal(std::string_view a, std::string_view b) { return a == b; }

}  // namespace gabby
