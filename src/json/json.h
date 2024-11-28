#ifndef GABBY_JSON_JSON_H_
#define GABBY_JSON_JSON_H_

#include <cstdio>
#include <format>
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
    NIL,
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
class NilValue;

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
    static ValuePtr Nil();

    // downcasts. specific type must implement their own
    virtual NumberValue& as_number() { throw TypeError(Type::NUM, type()); }
    virtual BooleanValue& as_boolean() { throw TypeError(Type::BOOL, type()); }
    virtual ObjectValue& as_object() { throw TypeError(Type::OBJ, type()); }
    virtual StringValue& as_string() { throw TypeError(Type::STR, type()); }
    virtual ArrayValue& as_array() { throw TypeError(Type::ARRAY, type()); }
    virtual NilValue& as_nil() { throw TypeError(Type::NIL, type()); }

    const NumberValue& as_number() const { return as_number(); }
    const BooleanValue& as_boolean() const { return as_boolean(); }
    const ObjectValue& as_object() const { return as_object(); }
    const StringValue& as_string() const { return as_string(); }
    const ArrayValue& as_array() const { return as_array(); }
    const NilValue& as_nil() const { return as_nil(); }

    // specific types must override their equals function
    virtual bool eq(const Value& other) const = 0;
    virtual bool eq(const NumberValue&) const { return false; }
    virtual bool eq(const BooleanValue&) const { return false; }
    virtual bool eq(const StringValue&) const { return false; }
    virtual bool eq(const ArrayValue&) const { return false; }
    virtual bool eq(const ObjectValue&) const { return false; }
    virtual bool eq(const NilValue&) const { return false; }
};

std::ostream& operator<<(std::ostream& os, const Value& value);
std::string to_string(const Value& value);
bool operator==(const Value& lhs, const Value& rhs);

template <typename T, Type typeVal>
class AbstractValue : public Value {
public:
    Type type() const override { return typeVal; }
    T& get() { return value_; }
    const T& get() const { return value_; }

protected:
    explicit AbstractValue(T value) : value_(value) {}

private:
    T value_;
};

class NilValue : public Value {
public:
    Type type() const override { return Type::NIL; }
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const NilValue& other) const override { return true; }
    NilValue& as_nil() override { return *this; }
    std::ostream& print(std::ostream& os) const override {
        return os << "null";
    }

protected:
    friend class Value;
};

class BooleanValue : public AbstractValue<bool, Type::BOOL> {
public:
    bool eq(const Value& other) const override { return other.eq(*this); }
    bool eq(const BooleanValue& other) const override {
        return get() == other.get();
    }
    BooleanValue& as_boolean() override { return *this; }
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
    NumberValue& as_number() override { return *this; }
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
    StringValue& as_string() override { return *this; }
    std::ostream& print(std::ostream& os) const override {
        return os << '"' << get() << '"';
    }

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
    ArrayValue& as_array() override { return *this; }
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

    ValuePtr operator[](size_t i) {
        if (i >= get().size()) {
            throw std::out_of_range(std::format("out of range: {}", i));
        }
        return get()[i];
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
    ObjectValue& as_object() override { return *this; }
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

    std::optional<ValuePtr> Lookup(const std::string& key) const {
        auto it = get().find(key);
        if (it == get().end()) return {};
        return it->second;
    }

    ValuePtr operator[](const std::string& key) { return get()[key]; }

protected:
    ObjectValue(std::unordered_map<std::string, ValuePtr> values)
        : AbstractValue(values) {}
    friend class Value;
};

}  // namespace json
}  // namespace gabby

#endif  // GABBY_JSON_JSON_H_
