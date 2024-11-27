#ifndef GABBY_TEST_H_
#define GABBY_TEST_H_

#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "utils/logging.h"

namespace gabby {

class TestCase {
public:
    virtual ~TestCase() {}
    virtual const std::string& suite() = 0;
    virtual const std::string& testcase() = 0;

    bool success() { return failures_ == 0; }

    void RunSafe() {
        try {
            Run();
        } catch (std::exception& e) {
            std::cerr << std::format("TEST: {}:{}: FAIL: got exception: {}\n",
                                     suite(), testcase(), e.what());
            fail();
        }
        if (failures_ == 0) {
            std::cout << std::format("TEST: {}:{}: ok\n", suite(), testcase());
        } else {
            std::cout << std::format("TEST: {}:{}: failure!\n", suite(),
                                     testcase());
        }
    }

protected:
    void fail() { failures_++; }
    virtual void Run() = 0;

private:
    int failures_ = 0;
};

extern std::vector<TestCase*>* kTestCases;

#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)

#define TEST(Suite, Case)                                        \
    class CONCAT(CONCAT(Test, Suite), Case) : public TestCase {  \
    public:                                                      \
        CONCAT(CONCAT(Test, Suite), Case)() {                    \
            if (kTestCases == nullptr)                           \
                kTestCases = new std::vector<TestCase*>;         \
            kTestCases->push_back(this);                         \
        }                                                        \
        void Run() override;                                     \
        const std::string& suite() override { return suite_; }   \
        const std::string& testcase() override { return case_; } \
                                                                 \
    private:                                                     \
        std::string suite_ = #Suite;                             \
        std::string case_ = #Case;                               \
    };                                                           \
    CONCAT(CONCAT(Test, Suite), Case)                            \
    CONCAT(register_, CONCAT(CONCAT(Test, Suite), Case));        \
    void CONCAT(CONCAT(Test, Suite), Case)::Run()

#define EXPECT_TRUE(want)                                                  \
    if (!(want)) {                                                         \
        std::cerr << std::format("TEST: {}:{}: FAIL: {}\n", suite_, case_, \
                                 #want);                                   \
        fail();                                                            \
    }

#define EXPECT_FALSE(want)                                                     \
    if ((want)) {                                                              \
        std::cerr << std::format("TEST: {}:{}: FAIL: wanted NOT {}\n", suite_, \
                                 case_, #want);                                \
        fail();                                                                \
    }

bool equal(std::string_view a, std::string_view b);

template <typename T, typename U>
bool equal(const T& t, const U& u) {
    return t == u;
}

#define EXPECT_EQ(got, want)                                              \
    if (!equal((got), (want))) {                                          \
        std::cerr << std::format("TEST: {}:{}: FAIL: {} != {}\n", suite_, \
                                 case_, #want, #got);                     \
        fail();                                                           \
    }

#define EXPECT_SUBSTR(haystack, needle) \
    EXPECT_TRUE((haystack).find(needle) >= 0)

}  // namespace gabby

#endif  // GABBY_TEST_H_
