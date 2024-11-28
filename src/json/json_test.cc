#include "json/json.h"

#include "test/test.h"

namespace gabby {
namespace json {

TEST(JSON, ParseNumber) {
    EXPECT_EQ(*Value::Number(0), *Value::Parse("0"));
    EXPECT_EQ(*Value::Number(17), *Value::Parse("17"));
    EXPECT_EQ(*Value::Number(-32.4), *Value::Parse("-32.4"));
}

TEST(JSON, ParseBoolean) {
    EXPECT_EQ(*Value::Boolean(true), *Value::Parse("17"));
    EXPECT_EQ(*Value::Boolean(false), *Value::Parse("0"));
}

TEST(JSON, ParseString) {
    EXPECT_EQ(*Value::String(""), *Value::Parse("\"\""));
    EXPECT_EQ(*Value::String("foo bar"), *Value::Parse("\"foo bar\""));
}

TEST(JSON, ParseArray) {
    EXPECT_EQ(*Value::Array({}), *Value::Parse("[]"));
    EXPECT_EQ(*Value::Array({Value::Boolean(true), Value::String("abc")}),
              *Value::Parse("[true, \"abc\"]"));
    EXPECT_EQ(*Value::Array({Value::Array({}), Value::Array({})}),
              *Value::Parse("[[],[]]"));
}

TEST(JSON, ParseObject) {
    EXPECT_EQ(*Value::Object({}), *Value::Parse("{}"));
    EXPECT_EQ(*Value::Object({
                  {std::string("a"), Value::String("b")},
                  {std::string("c"), Value::Number(1)},
              }),
              *Value::Parse("{\"a\": \"b\", \"c\": 1}"));
}

TEST(JSON, ParseCompletionRequest) {
    EXPECT_EQ(
        *Value::Object({
            {"model", Value::String("gabby-1")},
            {"messages",
             Value::Array({Value::Object({
                               {"role", Value::String("system")},
                               {"content",
                                Value::String("You are a helpful assistant.")},
                           }),
                           Value::Object({
                               {"role", Value::String("user")},
                               {"content", Value::String("Hello!")},
                           })})},
            {"stream", Value::Boolean(true)},
        }),
        *Value::Parse(R"(
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
    )"));
}

}  // namespace json
}  // namespace gabby
