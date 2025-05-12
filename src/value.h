#ifndef VALUE_H
#define VALUE_H
#include <string>
#include <vector>
#include <memory>

class Value{
public:
    virtual ~Value()=default;
    virtual std::string toString()const = 0;
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
};

class PairValue:public Value{
private:
    ValuePtr l; 
    ValuePtr r;
public:
    PairValue(ValuePtr l,ValuePtr r);
    std::string toString()const override;
};

#endif