#ifndef VALUE_H
#define VALUE_H
#include <string>
#include <vector>
#include <memory>
#include <optional>

class Value{
public:
    virtual ~Value()=default;
    virtual std::string toString()const = 0;
    bool isSelfEvaluating();
    bool isNil();
    bool isPair();
    bool isNumber();
    bool isString();
    bool isBoolean();
    bool isSymbol();
    bool isList();
    bool isProcedure();
    virtual int asNumber();
    virtual std::optional<std::string> asSymbol();
    virtual std::string asString();
    std::vector<std::shared_ptr<Value>> toVector();
    virtual bool isLispFalse();
};

using ValuePtr = std::shared_ptr<Value>;

class BooleanValue:public Value{
private:
    bool value;
public:
    BooleanValue(bool value);
    std::string toString()const override;
    bool isLispFalse()override;
};

class NumericValue:public Value{
private:
    double value;
public:
    NumericValue(double value);
    std::string toString()const override;
    int asNumber()override;
};

class StringValue:public Value{
private:
    std::string value;
public:
    StringValue(const std::string& value);
    std::string toString()const override;
    std::string asString()override;
};

class NilValue:public Value{
public:
    std::string toString()const override;
};

class SymbolValue : public Value {
private:
    std::string name;
public:
    SymbolValue(const std::string& name);
    std::string toString()const override;
    std::optional<std::string> asSymbol()override;
};

class PairValue:public Value{
public:
    ValuePtr l;
    ValuePtr r;
    PairValue(ValuePtr l,ValuePtr r);
    std::string toString()const override;
};

using BuiltinFuncType = ValuePtr(const std::vector<ValuePtr>&);

class BuiltinProcValue : public Value {
    BuiltinFuncType* func;
public:
    BuiltinProcValue(BuiltinFuncType* func):func{func}{}
    std::string toString()const override;
    BuiltinFuncType* get_function_pointer(){
        return func;
    }
};

class LambdaValue : public Value {
private:
    std::vector<std::string> params;
    std::vector<ValuePtr> body;
    // [...]
public:
    std::string toString() const override; 
};
  
ValuePtr toList(std::vector<ValuePtr>& params);

#endif