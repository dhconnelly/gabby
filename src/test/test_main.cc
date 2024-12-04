#include <mutex>

#include "inference/config.h"
#include "test/env.h"
#include "test/test.h"

namespace gabby {

static inference::InferenceConfig* kGlobalConfig = nullptr;
std::once_flag once;

const inference::InferenceConfig& GlobalConfig() {
    std::call_once(once, [] {
        kGlobalConfig =
            inference::LoadConfig(inference::FindDefaultModelDir()).release();
    });
    return *kGlobalConfig;
}

}  // namespace gabby

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
