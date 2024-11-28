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
    virtual std::ostream& print(std::ostream& os) const = 0;

    static ValuePtr Boolean(bool value);
    static ValuePtr String(std::string value);
    static ValuePtr Number(double value);
    static ValuePtr Array(std::vector<ValuePtr> values);
    static ValuePtr Object(std::unordered_map<std::string, ValuePtr> values);

    static ValuePtr Parse(FILE* f);
    static ValuePtr Parse(const std::string_view s);

    // downcasts. specific type must implement their own
    virtual const NumberValue& as_number() const {
        throw TypeError(Type::NUM, type());
    }
    virtual const BooleanValue& as_boolean() const {
        throw TypeError(Type::BOOL, type());
    }
    virtual const ObjectValue& as_object() const {
        throw TypeError(Type::OBJ, type());
    }
    virtual const StringValue& as_string() const {
        throw TypeError(Type::STR, type());
    }
    virtual const ArrayValue& as_array() const {
        throw TypeError(Type::ARRAY, type());
    }

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
    Type type() const override { return typeVal; }
    const T& get() const { return value_; }

protected:
    explicit AbstractValue(T value) : value_(value) {}

private:
    T value_;
};

// TODO: I think all of this can be lifted into the template.
// Try it after implementing parsing.

class BooleanValue : public AbstractValue<bool, Type::BOOL> {
public:
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const BooleanValue& other) const override {
        return get() == other.get();
    }
    const BooleanValue& as_boolean() const override { return *this; }
    std::ostream& print(std::ostream& os) const override {
        return os << (get() ? "true" : "false");
    }

protected:
    using AbstractValue::AbstractValue;
    friend class Value;
};

class NumberValue : public AbstractValue<double, Type::NUM> {
public:
    explicit NumberValue(int value) : AbstractValue((double)value) {}
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const NumberValue& other) const override {
        return get() == other.get();
    }
    const NumberValue& as_number() const override { return *this; }
    std::ostream& print(std::ostream& os) const override { return os << get(); }

protected:
    using AbstractValue::AbstractValue;
    friend class Value;
};

class StringValue : public AbstractValue<std::string, Type::STR> {
public:
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const StringValue& other) const override {
        return get() == other.get();
    }
    const StringValue& as_string() const override { return *this; }
    std::ostream& print(std::ostream& os) const override { return os << get(); }

protected:
    using AbstractValue::AbstractValue;
    friend class Value;
};

class ArrayValue : public AbstractValue<std::vector<ValuePtr>, Type::ARRAY> {
public:
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const ArrayValue& other) const override {
        if (get().size() != other.get().size()) return false;
        for (int i = 0; i < get().size(); i++) {
            if (*get()[i] != *other.get()[i]) return false;
        }
        return true;
    }
    const ArrayValue& as_array() const override { return *this; }
    std::ostream& print(std::ostream& os) const override {
        os << "[";
        bool first = true;
        for (const ValuePtr& value : get()) {
            if (!first) os << ", ";
            os << *value;
            first = false;
        }
        return os << "]";
    }

protected:
    ArrayValue(std::vector<ValuePtr> values) : AbstractValue(values) {}
    friend class Value;
};

class ObjectValue
    : public AbstractValue<std::unordered_map<std::string, ValuePtr>,
                           Type::OBJ> {
public:
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const ObjectValue& other) const override {
        if (get().size() != other.get().size()) return false;
        for (const auto& [k, v] : get()) {
            auto it = other.get().find(k);
            if (it == other.get().end() || *it->second != *v) return false;
        }
        return true;
    }
    const ObjectValue& as_object() const override { return *this; }
    std::ostream& print(std::ostream& os) const override {
        os << "{";
        bool first = true;
        for (const auto& [k, v] : get()) {
            if (!first) os << ", ";
            os << '"' << k << "\": " << *v;
            first = false;
        }
        return os << "}";
    }

protected:
    ObjectValue(std::unordered_map<std::string, ValuePtr> values)
        : AbstractValue(values) {}
    friend class Value;
};

}  // namespace json
}  // namespace gabby

#endif  // GABBY_JSON_JSON_H_
