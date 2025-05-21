#include "value.h"
#include "error.h"

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
        } 
        else if (typeid(*temp) == typeid(PairValue)) {
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
    return typeid(*this) == typeid(BooleanValue) || typeid(*this) == typeid(NumericValue) || typeid(*this) == typeid(BuiltinProcValue) || typeid(*this) == typeid(StringValue);
}

bool Value::isNil() {
    return typeid(*this) == typeid(NilValue);
}

bool Value::isPair() {
    return typeid(*this) == typeid(PairValue);
}

bool Value::isBoolean() {
    return typeid(*this) == typeid(BooleanValue);
}

bool Value::isSymbol() {
    return typeid(*this) == typeid(SymbolValue);
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

LambdaValue::LambdaValue(std::string name,
                         const std::vector<std::string>& params,
                         const std::vector<ValuePtr>& body,
                         std::shared_ptr<EvalEnv> env)
    : name(std::move(name)),
      params(params),
      body(body),
      captured_env(std::move(env)) {
    // Constructor body, if any
}

std::string LambdaValue::toString() const {
    return "#<procedure>";
}

const std::vector<std::string>& LambdaValue::get_params() const {
    return params;
}

const std::vector<ValuePtr>& LambdaValue::get_body() const {
    return body;
}

std::shared_ptr<EvalEnv> LambdaValue::get_captured_env() const {
    return captured_env;
}
bool Value::isNumber(){
    return typeid(*this) == typeid(NumericValue);
}

bool Value::isString(){
    return typeid(*this) == typeid(StringValue);
}

bool Value::isList(){
    if(!this->isPair()){
        return false;
    }
    PairValue* current = static_cast<PairValue*>(this);
    while(true) {
        if(current->r->isNil()){
            return true;
        }
        else if(current->r->isPair()){
            current = static_cast<PairValue*>(current->r.get());
        }
        else {
            return false;
        }
    }
}

bool Value::isProcedure(){
    return typeid(*this) == typeid(BuiltinProcValue);
}

double Value::asNumber(){
    throw LispError("Not a NumericValue");
}

double NumericValue::asNumber(){
    return value!=static_cast<int>(value)?value:static_cast<int>(value);
}

bool Value::isLispFalse(){
    return false;
}

bool BooleanValue::isLispFalse(){
    return !value;
}

std::string Value::asString(){
    throw LispError("Not a StringValue");
}

std::string StringValue::asString(){
    return value;
}

ValuePtr toList(std::vector<ValuePtr>& params){
    if (params.empty()){
        return std::make_shared<NilValue>();
    }
    auto head = std::make_shared<PairValue>(params.front(), std::make_shared<NilValue>());
    ValuePtr current = head;
    params.erase(params.begin());
    while(!params.empty()){
        auto newPair = std::make_shared<PairValue>(params.front(), std::make_shared<NilValue>());
        static_cast<PairValue*>(current.get())->r = newPair;
        current = newPair;
        params.erase(params.begin());
    }
    return head;
}

bool Value::getboolValue(){
    throw LispError("Not a BooleanValue");
}

bool BooleanValue::getboolValue(){
    return value;
}
