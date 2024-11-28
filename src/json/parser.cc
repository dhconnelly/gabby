#include "json/parser.h"

#include <cassert>
#include <format>

#include "utils/logging.h"

namespace gabby {
namespace json {

constexpr const std::string_view to_string(TokenType type) {
    using enum TokenType;
    switch (type) {
        case NUM: return "NUM";
        case STR: return "STR";
        case BOOL: return "BOOL";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case COMMA: return "COMMA";
        case COLON: return "COLON";
    }
    assert(false);
}

void Scanner::SkipWhitespace() {
    while (true) {
        int c = fgetc(f_);
        if (c == EOF) return;
        if (!isspace(c)) {
            assert(ungetc(c, f_) == c);
            return;
        }
    }
}

std::optional<char> Scanner::Peek() {
    int c = fgetc(f_);
    if (c == EOF) {
        if (feof(f_)) return {};
        if (errno == EAGAIN || errno == EWOULDBLOCK) return {};
        throw SystemError(errno);
    }
    assert(ungetc(c, f_) == c);
    return c;
}

char Scanner::Advance() {
    int c = fgetc(f_);
    if (c == EOF) {
        if (feof(f_)) throw ParsingError("unexpected eof");
        throw SystemError(errno);
    }
    return c;
}

bool is_delim(std::optional<char> c) { return !c.has_value() || !isalnum(*c); }

std::optional<Token> Scanner::Scan() {
    using enum TokenType;
    SkipWhitespace();
    std::optional<char> maybe_c = Peek();
    if (!maybe_c.has_value()) return {};
    int c = *maybe_c;

    switch (c) {
        case '[': return Token{.type = LBRACKET, .cargo = Advance()};
        case ']': return Token{.type = RBRACKET, .cargo = Advance()};
        case '{': return Token{.type = LBRACE, .cargo = Advance()};
        case '}': return Token{.type = RBRACE, .cargo = Advance()};
        case ',': return Token{.type = COMMA, .cargo = Advance()};
        case ':': return Token{.type = COLON, .cargo = Advance()};

        case '"': {
            Advance();
            std::string s;
            while (true) {
                c = Advance();
                if (c == '"' || c == '\n') break;
                s.push_back(c);
            }
            if (c != '"') throw ParsingError("unterminated string");
            return Token{.type = STR, .cargo = s};
        }
    }

    if (c == '-' || isdigit(c)) {
        std::string s;

        // sign
        if (c == '-') s.push_back(Advance());

        // integral part
        do {
            s.push_back(Advance());
        } while (!is_delim(Peek()));

        // decimal
        if (maybe_c = Peek(); maybe_c.has_value() && *maybe_c == '.') {
            s.push_back(Advance());

            // fractional part
            while (!is_delim(Peek())) {
                s.push_back(Advance());
            }
        }

        // parse
        double value;
        try {
            value = std::stod(s);
        } catch (...) {
            throw ParsingError("bad number: " + s);
        }

        return Token{.type = NUM, .cargo = value};
    }

    if (c == 't' || c == 'f') {
        std::string s;
        do {
            s.push_back(Advance());
        } while (!is_delim(Peek()));
        if (s != "true" && s != "false") {
            throw ParsingError("invalid boolean: " + s);
        }
        return Token{.type = BOOL, .cargo = (s == "true")};
    }

    throw ParsingError(std::format("bad token: {}", char(c)));
}

std::optional<Token> Parser::Peek() {
    if (!lookahead_.has_value()) lookahead_ = scan_.Scan();
    return lookahead_;
}

Token Parser::Next() {
    auto tok = Peek();
    lookahead_.reset();
    if (!tok.has_value()) throw ParsingError("unexpected eof");
    return *tok;
}

Token Parser::Eat(TokenType type) {
    Token tok = Next();
    if (tok.type != type) {
        throw ParsingError(std::format("want {}, got {}", to_string(type),
                                       to_string(tok.type)));
    }
    return tok;
}

ValuePtr Parser::Value() {
    using enum TokenType;
    std::optional<Token> tok = Peek();
    if (!tok.has_value()) throw ParsingError("unexpected eof");
    switch (tok->type) {
        case NUM: return Value::Number(std::get<double>(Next().cargo));
        case STR: return Value::String(std::get<std::string>(Next().cargo));
        case BOOL: return Value::Boolean(std::get<bool>(Next().cargo));

        case LBRACKET: {
            Eat(LBRACKET);
            std::vector<ValuePtr> values;
            while (true) {
                auto next = Peek();
                if (!next.has_value()) break;
                if (next->type == RBRACKET) break;
                if (!values.empty()) Eat(COMMA);
                values.push_back(Value());
            }
            Eat(RBRACKET);
            LOG(DEBUG) << "got " << values.size() << " values";
            return Value::Array(values);
        }

        case LBRACE: {
            Next();
            std::unordered_map<std::string, ValuePtr> values;
            while (true) {
                auto next = Peek();
                if (!next.has_value()) break;
                if (next->type == RBRACE) break;
                if (!values.empty()) Eat(COMMA);
                auto key = std::get<std::string>(Eat(STR).cargo);
                Eat(COLON);
                auto value = Value();
                values[key] = value;
            }
            Eat(RBRACE);
            return Value::Object(values);
        }

        default:
            throw ParsingError(
                std::format("bad value: {}", to_string(tok->type)));
    }
}

}  // namespace json
}  // namespace gabby
