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
        case NIL: return "NIL";
    }
    assert(false);
}

int Scanner::getc() {
    if (pos_ >= size_) return EOF;
    clearerr(f_);
    int c = fgetc(f_);
    if (c != EOF) ++pos_;
    return c;
}

void Scanner::ungetc(int c) {
    --pos_;
    assert(::ungetc(c, f_) == c);
}

void Scanner::SkipWhitespace() {
    while (true) {
        int c = getc();
        if (c == EOF) return;
        if (!isspace(c)) {
            ungetc(c);
            return;
        }
    }
}

std::optional<char> Scanner::Peek() {
    int c = getc();
    if (c == EOF) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return {};
        if (ferror(f_)) throw SystemError(errno);
        return {};
    }
    ungetc(c);
    return c;
}

char Scanner::Advance() {
    int c = getc();
    if (c == EOF) {
        if (ferror(f_)) throw SystemError(errno);
        else throw ParsingError("unexpected eof");
    }
    return c;
}

bool is_delim(std::optional<char> c) { return !c.has_value() || !isalnum(*c); }

std::optional<Token> Scanner::ScanSkipWhitespace() {
    auto tok = Scan();
    SkipWhitespace();
    return tok;
}

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

        // strings
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

    // numbers
    if (c == '-' || isdigit(c)) {
        std::string s;

        // sign
        if (c == '-') s.push_back(Advance());

        // integral part
        do {
            s.push_back(Advance());
        } while (!is_delim(Peek()) && *Peek() != 'e');

        // decimal
        if (Peek() == std::optional('.')) {
            s.push_back(Advance());

            // fractional part
            while (!is_delim(Peek()) && *Peek() != 'e') {
                s.push_back(Advance());
            }
        }

        // scientific notation
        if (Peek() == std::optional('e')) {
            s.push_back(Advance());

            // sign
            if (maybe_c = Peek();
                maybe_c == std::optional('+') || std::optional('-')) {
                s.push_back(Advance());
            }

            // exponent
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

    // null, booleans
    if (isalpha(c)) {
        std::string s;
        do {
            s.push_back(Advance());
        } while (!is_delim(Peek()));
        if (s == "true" || s == "false") {
            return Token{.type = BOOL, .cargo = (s == "true")};
        }
        if (s == "null") {
            return Token{.type = NIL, .cargo = "null"};
        }
    }

    throw ParsingError(std::format("bad token: {}", char(c)));
}

std::optional<Token> Parser::Peek() {
    if (!lookahead_.has_value()) lookahead_ = scan_.ScanSkipWhitespace();
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
        case NIL: {
            Next();
            return Value::Nil();
        }

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

ValuePtr Parse(FILE* f, int size) {
    auto parser = Parser(f, size);
    auto value = parser.Value();
    if (parser.pos() != size) throw ParsingError("unexpected trailing data");
    return value;
}

ValuePtr Parse(const std::string& s) {
    std::unique_ptr<FILE, decltype(&fclose)> f(
        fmemopen((void*)s.data(), s.size(), "r"), fclose);
    if (f.get() == nullptr) throw SystemError(errno);
    return Parse(f.get(), s.size());
}

}  // namespace json
}  // namespace gabby
