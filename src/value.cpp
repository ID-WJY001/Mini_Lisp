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
