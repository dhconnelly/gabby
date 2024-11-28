#ifndef GABBY_JSON_JSON_H_
#define GABBY_JSON_JSON_H_

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace gabby {
namespace json {

enum class Type {
    NUM,
    BOOL,
    STR,
    ARRAY,
    OBJ,
};

std::ostream& operator<<(std::ostream& os, Type type);
std::string to_string(Type type);

class JSONError : public ::std::runtime_error {
public:
    explicit JSONError(const std::string& s) : ::std::runtime_error(s) {}
};

class TypeError : public JSONError {
public:
    TypeError(Type want, Type got);
};

class ParsingError : public JSONError {
public:
    explicit ParsingError(const std::string& s) : JSONError(s) {}
};

class Value;
class NumberValue;
class BooleanValue;
class StringValue;
class ArrayValue;
class ObjectValue;

using ValuePtr = std::shared_ptr<Value>;

class Value {
public:
    virtual ~Value() {}
    virtual Type type() const = 0;

    static ValuePtr Boolean(bool value);
    static ValuePtr String(std::string value);
    static ValuePtr Number(double value);
    static ValuePtr Array(std::vector<ValuePtr> values);
    static ValuePtr Object(std::unordered_map<std::string, ValuePtr> values);

    static ValuePtr Parse(FILE* f);
    static ValuePtr Parse(const std::string_view s);

    // downcasts
    virtual NumberValue& as_number() { throw TypeError(Type::NUM, type()); }
    virtual BooleanValue& as_boolean() { throw TypeError(Type::BOOL, type()); }
    virtual ObjectValue& as_object() { throw TypeError(Type::OBJ, type()); }
    virtual StringValue& as_string() { throw TypeError(Type::STR, type()); }
    virtual ArrayValue& as_array() { throw TypeError(Type::ARRAY, type()); }
    const NumberValue& as_number() const { return as_number(); }
    const BooleanValue& as_boolean() const { return as_boolean(); }
    const ObjectValue& as_object() const { return as_object(); }
    const StringValue& as_string() const { return as_string(); }
    const ArrayValue& as_array() const { return as_array(); }

    // specific types must override their equals function
    virtual bool eq(const Value& other) const = 0;
    virtual bool eq(const NumberValue&) const { return false; }
    virtual bool eq(const BooleanValue&) const { return false; }
    virtual bool eq(const StringValue&) const { return false; }
    virtual bool eq(const ArrayValue&) const { return false; }
    virtual bool eq(const ObjectValue&) const { return false; }
};

std::ostream& operator<<(std::ostream& os, const Value& value);
std::string to_string(const Value& value);
bool operator==(const Value& lhs, const Value& rhs);

template <typename T, Type typeVal>
class AbstractValue : public Value {
public:
    explicit AbstractValue(T value) : value_(value) {}
    Type type() const override { return typeVal; }
    const T& get() const { return value_; }

private:
    T value_;
};

// TODO: I think all of this can be lifted into the template.
// Try it after implementing parsing.

class BooleanValue : public AbstractValue<bool, Type::BOOL> {
public:
    using AbstractValue::AbstractValue;
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const BooleanValue& other) const override {
        return get() == other.get();
    }
};

class NumberValue : public AbstractValue<double, Type::NUM> {
public:
    using AbstractValue::AbstractValue;
    explicit NumberValue(int value) : AbstractValue((double)value) {}
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const NumberValue& other) const override {
        return get() == other.get();
    }
};

class StringValue : public AbstractValue<std::string, Type::STR> {
public:
    using AbstractValue::AbstractValue;
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const StringValue& other) const override {
        return get() == other.get();
    }
};

class ArrayValue : public AbstractValue<std::vector<ValuePtr>, Type::ARRAY> {
public:
    ArrayValue(std::vector<ValuePtr> values) : AbstractValue(values) {}

    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const ArrayValue& other) const override {
        if (get().size() != other.get().size()) return false;
        for (int i = 0; i < get().size(); i++) {
            if (get()[i] != other.get()[i]) return false;
        }
        return true;
    }
};

class ObjectValue
    : public AbstractValue<std::unordered_map<std::string, ValuePtr>,
                           Type::OBJ> {
public:
    ObjectValue(std::unordered_map<std::string, ValuePtr> values)
        : AbstractValue(values) {}

    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const ObjectValue& other) const override {
        if (get().size() != other.get().size()) return false;
        for (const auto& [k, v] : get()) {
            auto it = other.get().find(k);
            if (it == other.get().end() || it->second != v) return false;
        }
        return true;
    }
};

}  // namespace json
}  // namespace gabby

#endif  // GABBY_JSON_JSON_H_
