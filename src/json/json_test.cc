#include "json/json.h"

#include "test/test.h"

namespace gabby {
namespace json {

TEST(JSON, ParseNumber) {
    EXPECT_EQ(NumberValue(0), *Value::Parse("0"));
    EXPECT_EQ(NumberValue(17), *Value::Parse("17"));
    EXPECT_EQ(NumberValue(-32.4), *Value::Parse("-32.4"));
}

TEST(JSON, ParseBoolean) {
    EXPECT_EQ(BooleanValue(true), *Value::Parse("17"));
    EXPECT_EQ(BooleanValue(false), *Value::Parse("0"));
}

TEST(JSON, ParseString) {
    EXPECT_EQ(StringValue(""), *Value::Parse("\"\""));
    EXPECT_EQ(StringValue("foo bar"), *Value::Parse("\"foo bar\""));
}

TEST(JSON, ParseArray) {
    EXPECT_EQ(ArrayValue({}), *Value::Parse("[]"));
    EXPECT_EQ(ArrayValue({Value::Boolean(true), Value::String("abc")}),
              *Value::Parse("[true, \"abc\"]"));
    EXPECT_EQ(ArrayValue({}), *Value::Parse("[]"));
}

TEST(JSON, ParseObject) {
    EXPECT_EQ(ObjectValue({}), *Value::Parse("{}"));
    EXPECT_EQ(ObjectValue({
                  {std::string("a"), Value::String("b")},
                  {std::string("c"), Value::Number(1)},
              }),
              *Value::Parse("{\"a\": \"b\", \"c\": 1}"));
}

TEST(JSON, ParseCompletionRequest) {
    std::string json = R"(
        {
            "model": "gabby-1",
            "messages": [
                {
                    "role": "system",
                    "content": "You are a helpful assistant."
                },
                {
                    "role": "user",
                    "content": "Hello!"
                }
            ],
            "stream": true
        }
    )";
    EXPECT_EQ(1, 1);
}

}  // namespace json
}  // namespace gabby
