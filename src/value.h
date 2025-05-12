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
    virtual std::optional<std::string> asSymbol();
    std::vector<std::shared_ptr<Value>> toVector();
};

using ValuePtr = std::shared_ptr<Value>;

class BooleanValue:public Value{
private:
    bool value;
public:
    BooleanValue(bool value);
    std::string toString()const override;
};

class NumericValue:public Value{
private:
    double value;
public:
    NumericValue(double value);
    std::string toString()const override;
};

class StringValue:public Value{
private:
    std::string value;
public:
    StringValue(const std::string& value);
    std::string toString()const override;
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

#endif