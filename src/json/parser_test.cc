#include "json/parser.h"

#include "json/json.h"
#include "test/test.h"

namespace gabby {
namespace json {

TEST(JSON, ParseNull) { EXPECT_EQ(*Value::Nil(), *Parse("null")); }

TEST(JSON, ParseNumber) {
    EXPECT_EQ(*Value::Number(0), *Parse("0"));
    EXPECT_EQ(*Value::Number(17), *Parse("17"));
    EXPECT_FLOAT_EQ(Value::Number(-32.4)->as_number().get(),
                    Parse("-32.4")->as_number().get(), 0.001);
    EXPECT_FLOAT_EQ(Value::Number(1e-17)->as_number().get(),
                    Parse("1e-17")->as_number().get(), 0.000000000001);
}

TEST(JSON, ParseBoolean) {
    EXPECT_EQ(*Value::Boolean(true), *Parse("true"));
    EXPECT_EQ(*Value::Boolean(false), *Parse("false"));
}

TEST(JSON, ParseString) {
    EXPECT_EQ(*Value::String(""), *Parse("\"\""));
    EXPECT_EQ(*Value::String("foo bar"), *Parse("\"foo bar\""));
}

TEST(JSON, ParseArray) {
    EXPECT_EQ(*Value::Array({}), *Parse("[]"));
    EXPECT_EQ(*Value::Array({Value::Boolean(true), Value::String("abc")}),
              *Parse("[true, \"abc\"]"));
    EXPECT_EQ(*Value::Array({Value::Array({}), Value::Array({})}),
              *Parse("[[],[]]"));
}

TEST(JSON, ParseObject) {
    EXPECT_EQ(*Value::Object({}), *Parse("{}"));
    EXPECT_EQ(*Value::Object({
                  {std::string("a"), Value::String("b")},
                  {std::string("c"), Value::Number(1)},
              }),
              *Parse("{\"a\": \"b\", \"c\": 1}"));
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
        *Parse(R"(
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
