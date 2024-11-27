#include "test/test.h"

int main(int argc, char* argv[]) {
    int failures = 0;
    std::cout << "running " << gabby::kTestCases->size() << " tests\n";
    for (auto* test_case : *gabby::kTestCases) {
        test_case->RunSafe();
        if (!test_case->success()) failures++;
    }
    if (failures == 0) {
        std::cout << "succcess\n";
    } else {
        std::cout << failures << " failures\n";
    }
    return (failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
