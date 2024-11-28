#include "json/json.h"

#include <cassert>
#include <cstdio>
#include <format>
#include <sstream>

#include "json/parser.h"
#include "utils/logging.h"

namespace gabby {
namespace json {

std::ostream& operator<<(std::ostream& os, Type type) {
    switch (type) {
        case Type::NUM: return os << "NUM";
        case Type::BOOL: return os << "BOOL";
        case Type::STR: return os << "BOOL";
        case Type::ARRAY: return os << "ARRAY";
        case Type::OBJ: return os << "OBJ";
    }
    assert(false);
}

std::string to_string(Type type) {
    std::stringstream ss;
    ss << type;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Value& value) {
    return value.print(os);
}

std::string to_string(const Value& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

TypeError::TypeError(Type want, Type got)
    : JSONError(std::format("wrong type: want {}, got {}", to_string(want),
                            to_string(got))) {}

/* static */
ValuePtr Value::Parse(FILE* f) { return Parser(f).Value(); }

/* static */
ValuePtr Value::Parse(const std::string_view s) {
    std::unique_ptr<FILE, decltype(&fclose)> f(
        fmemopen((void*)s.data(), s.size(), "r"), fclose);
    if (f.get() == nullptr) throw SystemError(errno);
    return Parse(f.get());
}

bool operator==(const Value& lhs, const Value& rhs) { return lhs.eq(rhs); }

ValuePtr Value::Boolean(bool value) {
    return std::shared_ptr<Value>(new BooleanValue(value));
}
ValuePtr Value::String(std::string value) {
    return std::shared_ptr<Value>(new StringValue(value));
}
ValuePtr Value::Number(double value) {
    return std::shared_ptr<Value>(new NumberValue(value));
}
ValuePtr Value::Array(std::vector<ValuePtr> values) {
    return std::shared_ptr<Value>(new ArrayValue(values));
}
ValuePtr Value::Object(std::unordered_map<std::string, ValuePtr> values) {
    return std::shared_ptr<Value>(new ObjectValue(values));
}

}  // namespace json
}  // namespace gabby
