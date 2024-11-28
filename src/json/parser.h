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
    Scanner(FILE* f, int size) : f_(f), size_(size) {}
    std::optional<Token> ScanSkipWhitespace();
    int pos() const { return pos_; }

private:
    void SkipWhitespace();
    std::optional<char> Peek();
    char Advance();
    int getc();
    void ungetc(int c);
    std::optional<Token> Scan();

    FILE* f_;
    int size_;
    int pos_ = 0;
};

class Parser {
public:
    Parser(FILE* f, int size) : scan_(Scanner(f, size)) {}

    ValuePtr Value();
    int pos() const { return scan_.pos(); }

private:
    std::optional<Token> Peek();
    Token Next();
    Token Eat(TokenType type);

    Scanner scan_;
    std::optional<Token> lookahead_;
};

ValuePtr Parse(FILE* f, int size);
ValuePtr Parse(const std::string_view s);

}  // namespace json
}  // namespace gabby

#endif  // GABBY_JSON_PARSER_H_
