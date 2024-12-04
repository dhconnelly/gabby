#include "inference/tokenizer.h"

#include "test/env.h"
#include "test/test.h"

namespace gabby {
namespace inference {

TEST(Tokenizer, Empty) {
    const auto& config = GlobalConfig();
    Tokenizer tok(config.special_tokens_map, config.tok_config, config.tok);
    EXPECT_EQ(std::vector<int>{}, tok.Tokenize(""));
}

TEST(Tokenizer, Tokenize) {
    const auto& config = GlobalConfig();
    Tokenizer tok(config.special_tokens_map, config.tok_config, config.tok);
    EXPECT_EQ(std::vector<int>{}, tok.Tokenize(""));
}

TEST(Tokenizer, SpecialTokens) {
    const auto& config = GlobalConfig();
    Tokenizer tok(config.special_tokens_map, config.tok_config, config.tok);
    EXPECT_EQ(std::vector<int>{}, tok.Tokenize(""));
}

}  // namespace inference
}  // namespace gabby
