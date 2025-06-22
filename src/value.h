#ifndef VALUE_H
#define VALUE_H
#include <string>
#include <vector>
#include <memory>
#include <optional>

class EvalEnv;

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
    bool isLambda();
    virtual double asNumber();
    virtual std::optional<std::string> asSymbol();
    virtual std::string asString();
    std::vector<std::shared_ptr<Value>> toVector();
    virtual bool isLispFalse();
    virtual bool getboolValue();
};

using ValuePtr = std::shared_ptr<Value>;

class BooleanValue:public Value{
private:
    bool value;
public:
    bool getboolValue()override;
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
    double asNumber()override;
    double getValue() const { return value; }
};

class StringValue:public Value{
private:
    std::string value;
public:
    StringValue(const std::string& value);
    std::string toString()const override;
    std::string asString()override;
    const std::string& getValue() const { return value; }
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

using BuiltinFuncType = ValuePtr (*)(const std::vector<ValuePtr>& args, EvalEnv& env);

class BuiltinProcValue : public Value {
    BuiltinFuncType func;
public:
    BuiltinProcValue(BuiltinFuncType func):func{func}{}
    std::string toString()const override;
    BuiltinFuncType get_function_pointer(){
        return func;
    }
};

class LambdaValue : public Value {
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<ValuePtr> body;
    std::shared_ptr<EvalEnv> captured_env;
    LambdaValue(std::string name, const std::vector<std::string>& params, const std::vector<ValuePtr>& body, std::shared_ptr<EvalEnv> env);
    std::string toString() const override; 
    const std::vector<std::string>& get_params() const;
    const std::vector<ValuePtr>& get_body() const;
    std::shared_ptr<EvalEnv> get_captured_env() const;
};
class RationalValue : public Value {
private:
    int numerator;
    int denominator;
    void reduce(); // 约分
public:
    RationalValue(int num, int denom);
    int getNumerator() const { return numerator; }
    int getDenominator() const { return denominator; }
    std::string toString() const override;
    double asNumber() override;
};
class MacroValue : public Value {
public:
    std::vector<std::string> params;
    ValuePtr body;
    MacroValue(const std::vector<std::string>& params, ValuePtr body)
        : params(params), body(body) {}
    std::string toString() const override {
        return "#<macro>";
    }
};

ValuePtr toList(std::vector<ValuePtr>& params);

#endif