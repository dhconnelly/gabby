#ifndef GABBY_TEST_H_
#define GABBY_TEST_H_

#include <cmath>
#include <format>
#include <iostream>
#include <source_location>
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

    void error(const std::string_view message, std::source_location loc) {
        std::cerr << std::format("TEST: {}:{}: {}\n", loc.file_name(),
                                 loc.line(), message);
    }

    void error(const std::string_view message) {
        std::cerr << std::format("TEST: {}:{}: {}\n", suite(), testcase(),
                                 message);
    }

    void log(const std::string_view message) {
        std::cout << std::format("TEST: {}:{}: {}\n", suite(), testcase(),
                                 message);
    }

    void RunSafe() {
        try {
            Run();
        } catch (const std::exception& e) {
            error(std::format("caught exception: {}", e.what()));
            fail();
        } catch (...) {
            error("caught unknown exception");
            fail();
        }
        if (failures_ == 0) {
            log(std::format("ok", suite(), testcase()));
        } else {
            log(std::format("failure!", suite(), testcase()));
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

#define EXPECT_TRUE(want)                       \
    if (!(want)) {                              \
        error(std::format("FAIL: {}", #want),   \
              std::source_location::current()); \
        fail();                                 \
    }

#define EXPECT_FALSE(want)                       \
    if ((want)) {                                \
        error(std::format("FAIL: !({})", #want), \
              std::source_location::current());  \
        fail();                                  \
    }

#define EXPECT_EQ(got, want)                              \
    if ((got) != (want)) {                                \
        error(std::format("FAIL: {} != {}", #want, #got), \
              std::source_location::current());           \
        fail();                                           \
    }

#define EXPECT_FLOAT_EQ(got, want, eps)                   \
    if (abs((want) - (got) > eps)) {                      \
        error(std::format("FAIL: {} != {}", #want, #got), \
              std::source_location::current());           \
        fail();                                           \
    }

#define EXPECT_SUBSTR(haystack, needle) \
    EXPECT_TRUE((haystack).find(needle) >= 0)

}  // namespace gabby

#endif  // GABBY_TEST_H_
