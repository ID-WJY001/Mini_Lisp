#include "value.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>

BooleanValue::BooleanValue(bool value) : value(value) {}

std::string BooleanValue::toString() const {
    return value ? "#t" : "#f";
}   

NumericValue::NumericValue(double value) : value(value) {}

std::string NumericValue::toString() const {
    if (value==static_cast<int>(value)) {
        return std::to_string(static_cast<int>(value));
    }
    return std::to_string(value);
}

StringValue::StringValue(const std::string& value) : value(value) {}

std::string StringValue::toString() const {
    std::ostringstream oss;
    oss << std::quoted(value);
    return oss.str();
}

std::string NilValue::toString() const {
    return "()";
}

SymbolValue::SymbolValue(const std::string& name) : name(name) {}

std::string SymbolValue::toString() const {
    return name;
}

PairValue::PairValue(ValuePtr l, ValuePtr r) : l(l), r(r) {}

std::string PairValue::toString() const {
    std::string result = "(" + l->toString();
    ValuePtr temp = r;
    while (true) {
        if (typeid(*temp) == typeid(NilValue)) {
            return result + ")";
        } else if (typeid(*temp) == typeid(PairValue)) {
            auto pair = static_cast<const PairValue&>(*temp);
            result += " " + pair.l->toString();
            temp = pair.r;
        } 
        else {
            return result + " . " + temp->toString() + ")";
        }
    }
}

bool Value::isSelfEvaluating() {
    return typeid(*this) == typeid(BooleanValue) || 
           typeid(*this) == typeid(NumericValue) || 
           typeid(*this) == typeid(BuiltinProcValue) || 
           typeid(*this) == typeid(StringValue);
}

bool Value::isNil() {
    return typeid(*this) == typeid(NilValue);
}

bool Value::isPair() {
    return typeid(*this) == typeid(PairValue);
}

std::vector<ValuePtr> Value::toVector() {
    if (this->isNil()) {
        return {};
    }
    if (!this->isPair()) {
        throw std::runtime_error("Cannot convert non-list Value to vector. Value is not a Pair or Nil: " + this->toString());
    } 
    std::vector<ValuePtr> vec;
    Value* current_node_ptr = this;
    while (true) {
        if (typeid(*current_node_ptr) == typeid(PairValue)) {
            PairValue* pair = static_cast<PairValue*>(current_node_ptr);
            vec.push_back(pair->l); 
            current_node_ptr = pair->r.get();
        }
        else if (typeid(*current_node_ptr) == typeid(NilValue)) {
            break;
        }
        else {
            throw std::runtime_error("Cannot convert improper list to vector. List tail is not Nil or Pair: " + current_node_ptr->toString());
        }
    }
    return vec;
}

std::optional<std::string> Value::asSymbol(){
    return std::nullopt;
}

std::optional<std::string> SymbolValue::asSymbol(){
    return this->name; 
}

std::string BuiltinProcValue :: toString() const {
    return "#<procedure>";
}

bool Value::isNumber(){
    return typeid(*this) == typeid(NumericValue);
}

int Value::asNumber(){
    throw std::runtime_error("Not a NumericValue");
}

int NumericValue::asNumber(){
    return static_cast<int>(value);
}