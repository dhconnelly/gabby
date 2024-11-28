#ifndef GABBY_JSON_PARSER_H_
#define GABBY_JSON_PARSER_H_

#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "json/json.h"

namespace gabby {
namespace json {

enum class TokenType {
    NUM,
    STR,
    BOOL,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    COMMA,
    COLON,
};

struct Token {
    TokenType type;
    std::variant<double, bool, char, std::string> cargo;
};

class Scanner {
public:
    Scanner(FILE* f) : f_(f) {}
    std::optional<Token> Scan();

private:
    void SkipWhitespace();
    std::optional<char> Peek();
    char Advance();

    FILE* f_;
};

class Parser {
public:
    Parser(FILE* f) : scan_(Scanner(f)) {}

    ValuePtr Value();

private:
    std::optional<Token> Peek();
    Token Next();
    Token Eat(TokenType type);

    Scanner scan_;
    std::optional<Token> lookahead_;
};

}  // namespace json
}  // namespace gabby

#endif  // GABBY_JSON_PARSER_H_
