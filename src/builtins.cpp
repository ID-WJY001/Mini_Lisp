#include <iostream>
#include <iomanip>
#include <cmath>
#include "builtins.h"
#include "value.h"

static ValuePtr builtin_display(const std::vector<ValuePtr>& params) {
    if(params.size() != 1){
        throw LispError("Exactly 1 value can be displayed at a time.");
    }
    auto it = params[0];
    if(it->isString()){
        std::cout << it->asString();
    }
    else{
        std::cout << it->toString();
    }
    return std::make_shared<NilValue>();
}

static ValuePtr builtin_exit(const std::vector<ValuePtr>& params) {
    if(params.empty()){
        std::exit(0);
    }
    if(params.size() != 1){
        throw LispError("At most 1 value can exist in function exit.");
    }
    auto it = params[0];
    if(!it->isNumber() || it->asNumber() != static_cast<int>(it->asNumber())){
        throw LispError("Exit error.");
    }
    std::exit(it->asNumber());
    return std::make_shared<NilValue>();
}

static ValuePtr builtin_newline(const std::vector<ValuePtr>& params) {
    if(!params.empty()){
        throw LispError("No values should exist in function newline.");
    }
    std::cout<<std::endl;
    return std::make_shared<NilValue>();
}

static ValuePtr builtin_print(const std::vector<ValuePtr>& params) {
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i]) {
            std::cout << params[i]->toString();
        } 
        else {
            std::cout << "#<nullptr-in-print>";
        }
        if (i < params.size() - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return std::make_shared<NilValue>();
}

static ValuePtr builtin_atom(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isNil() || params[0]->isBoolean() || params[0]->isNumber() || params[0]->isString() || params[0]->isSymbol());
}

static ValuePtr builtin_boolean(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isBoolean());
}

static ValuePtr builtin_integer(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isNumber() && params[0]->asNumber() == static_cast<int>(params[0]->asNumber()));
}

static ValuePtr builtin_list_(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    auto it = std::static_pointer_cast<PairValue>(params[0]);
    if(!it->isPair()){
        return std::make_shared<BooleanValue>(0);
    }
    while(it->r->isPair() || it->r->isNil()){
        if(it->r->isNil()){
            return std::make_shared<BooleanValue>(1);
        }
        auto it_r = std::static_pointer_cast<PairValue>(it->r);
        it->l = it_r->l;
        it->r = it_r->r;
    }
    return std::make_shared<BooleanValue>(0);
}

static ValuePtr builtin_number(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isNumber());
}

static ValuePtr builtin_null(const std::vector<ValuePtr>& params) {
    if (params.size()!=1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isNil());
}

static ValuePtr builtin_pair(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isPair());
}

static ValuePtr builtin_procedure(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isProcedure());
}

static ValuePtr builtin_string(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isString());
}

static ValuePtr builtin_symbol(const std::vector<ValuePtr>& params) {
    if (params.size() != 1) {
        throw LispError("Exactly 1 value should be judged at a time.");
    }
    return std::make_shared<BooleanValue>(params[0]->isSymbol());
}

static ValuePtr builtin_append(const std::vector<ValuePtr>& params) {
    if(params.empty()) {
        throw LispError("No values in function append.");
    }
    std::vector<ValuePtr> new_list{};
    for(auto param : params){
        if(!param->isList()){
            throw LispError("Non-list values shouldnot exist in function append.");
        }
        auto param_vector = param->toVector();
        for(auto i : param_vector){
            new_list.push_back(i);
        }
    }
    auto appended_list = toList(new_list);
    return appended_list;
}

static ValuePtr builtin_car(const std::vector<ValuePtr>& params) {
    if(params.size() != 1) {
        throw LispError("Exactly 1 value should be calculated at a time.");
    }
    if(!params[0]->isPair()){
        throw LispError("Only PairValues have cars.");
    }
    auto param = std::static_pointer_cast<PairValue>(params[0]);
    return param->l;
}

static ValuePtr builtin_cdr(const std::vector<ValuePtr>& params) {
    if(params.size() != 1) {
        throw LispError("Exactly 1 value should be calculated at a time.");
    }
    if(!params[0]->isPair()){
        throw LispError("Only PairValues have cars.");
    }
    auto param = std::static_pointer_cast<PairValue>(params[0]);
    return param->r;
}

static ValuePtr builtin_cons(const std::vector<ValuePtr>& params) {
    if(params.size() != 2) {
        throw LispError("Exactly 2 value should exist in function cons.");
    }
    return std::make_shared<PairValue>(params[0], params[1]);
}

static ValuePtr builtin_length(const std::vector<ValuePtr>& params) {
    if(params.size() != 1) {
        throw LispError("Exactly 1 value should be calculated at a time.");
    }
    if(!params[0]->isList() && !params[0]->isNil()){
        throw LispError("Only lists or nils should exist in function length.");
    }
    int cnt = 0;
    auto param_vector = params[0]->toVector();
    cnt = param_vector.size();
    return std::make_shared<NumericValue>(cnt);
}

static ValuePtr builtin_list(const std::vector<ValuePtr>& params) {
    std::vector<ValuePtr> copy = params;
    return toList(copy);
}

static ValuePtr builtin_add(const std::vector<ValuePtr>& params) {
    double result = 0.0; 
    for (const auto& i : params) {
        if (!i->isNumber()) {
            throw LispError("Cannot add a non-numeric value.");
        }
        result += i->asNumber();
    }
    return std::make_shared<NumericValue>(result);
}

static ValuePtr builtin_subtract(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function subtraction.");
    }
    if (!params[0]->isNumber()) {
        throw LispError("A non-numeric value cannot be subtracted.");
    }
    double result = params[0]->asNumber();
    if (params.size() == 1) {
        return std::make_shared<NumericValue>(-result);
    }
    for (size_t i = 1; i < params.size(); ++i) {
        const auto& current_param = params[i];
        if (!current_param->isNumber()) {
            throw LispError("Cannot substract a non-numeric value.");
        }
        result -= current_param->asNumber();
    }
    return std::make_shared<NumericValue>(result);
}

static ValuePtr builtin_multiply(const std::vector<ValuePtr>& params) {
    double result = 1.0;
    for (const auto& i : params) {
        if (!i->isNumber()) {
            throw LispError("Cannot multiply a non-numeric value.");
        }
        result *= i->asNumber();
    }
    return std::make_shared<NumericValue>(result);
}

static ValuePtr builtin_divide(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function division.");
    }
    if (!params[0]->isNumber()) {
        throw LispError("Cannot divide a non-numeric value.");
    }
    double result = params[0]->asNumber();
    if (params.size() == 1) {
        if (result == 0.0) {
            throw LispError("0 cannot be divided.");
        }
        return std::make_shared<NumericValue>(1.0 / result);
    }
    for (size_t i = 1; i < params.size(); ++i) {
        const auto& current_param = params[i];
        if (!current_param->isNumber()) {
            throw LispError("Cannot divide a non-numeric value.");
        }
        double divisor = current_param->asNumber();
        if (divisor == 0.0) {
            throw LispError("0 cannot be divided.");
        }
        result /= divisor;
    }
    return std::make_shared<NumericValue>(result);
}

static ValuePtr builtin_abs(const std::vector<ValuePtr>& params) {
    if (params.size()!=1) {
        throw LispError("Only 1 value can be calculated at a time.");
    }
    if ((!params[0]->isNumber())) {
        throw LispError("Cannot calculate non-numeric values.");
    }
    double result = params[0]->asNumber();
    return std::make_shared<NumericValue>(abs(result));
}

static ValuePtr builtin_expr(const std::vector<ValuePtr>& params) {
    if (params.size()!=2) {
        throw LispError("Exactly 2 values should be calculated at a time.");
    }
    if (((!params[0]->isNumber())||(!params[1]->isNumber()))) {
        throw LispError("Cannot calculate non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    return std::make_shared<NumericValue>(pow(leftvalue,rightvalue));
}

static ValuePtr builtin_quotient(const std::vector<ValuePtr>& params) {
    if (params.size()!=2) {
        throw LispError("Exactly 2 values should be calculated at a time.");
    }
    if (((!params[0]->isNumber())||(!params[1]->isNumber()))) {
        throw LispError("Cannot calculate non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    if (rightvalue==0) {
        throw LispError("Cannot devide 0.");
    }
    return leftvalue * rightvalue >= 0 ? std::make_shared<NumericValue>(floor(leftvalue/rightvalue)) : std::make_shared<NumericValue>(ceil(leftvalue/rightvalue));
}

static ValuePtr builtin_modulo(const std::vector<ValuePtr>& params) {
    if (params.size() != 2) {
        throw LispError("Modulo function 'modulo' requires exactly 2 arguments.");
    }
    if (!params[0]->isNumber() || !params[1]->isNumber()) {
        throw LispError("Modulo function 'modulo' requires numeric arguments.");
    }
    double dividend_x = params[0]->asNumber();
    double divisor_y = params[1]->asNumber();
    if (divisor_y == 0.0) {
        throw LispError("Modulo by zero (divisor y cannot be zero).");
    }
    double result_q = dividend_x - divisor_y * std::floor(dividend_x / divisor_y);
    return std::make_shared<NumericValue>(result_q);
}

static ValuePtr builtin_remainder(const std::vector<ValuePtr>& params) {
    if (params.size() != 2) {
        throw LispError("Remainder function 'remainder' requires exactly 2 arguments.");
    }
    if (!params[0]->isNumber() || !params[1]->isNumber()) {
        throw LispError("Remainder function 'remainder' requires numeric arguments.");
    }
    double dividend_x = params[0]->asNumber();
    double divisor_y = params[1]->asNumber();
    if (divisor_y == 0.0) {
        throw LispError("Remainder by zero (divisor y cannot be zero).");
    }
    double result_q = std::fmod(dividend_x, divisor_y);
    return std::make_shared<NumericValue>(result_q);
}

static ValuePtr builtin_greater(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function greater.");
    }
    if (params.size()!=2) {
        throw LispError("Only 2 values can be compared at a time.");
    }
    if ((!params[0]->isNumber())||(!params[0]->isNumber())) {
        throw LispError("Cannot compare non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    return leftvalue > rightvalue ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_lesser(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function lesser.");
    }
    if (params.size()!=2) {
        throw LispError("Only 2 values can be compared at a time.");
    }
    if ((!params[0]->isNumber())||(!params[0]->isNumber())) {
        throw LispError("Cannot compare non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    return leftvalue < rightvalue ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_equal(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function equal.");
    }
    if (params.size()!=2) {
        throw LispError("Only 2 values can be compared at a time.");
    }
    if ((!params[0]->isNumber())||(!params[0]->isNumber())) {
        throw LispError("Cannot compare non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    return leftvalue == rightvalue ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_greater_equal(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function greater_equal.");
    }
    if (params.size()!=2) {
        throw LispError("Only 2 values can be compared at a time.");
    }
    if ((!params[0]->isNumber())||(!params[0]->isNumber())) {
        throw LispError("Cannot compare non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    return leftvalue >= rightvalue ? std::make_shared<BooleanValue>(true) :  std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_lesser_equal(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function lesser_equal.");
    }
    if (params.size()!=2) {
        throw LispError("Only 2 values can be compared at a time.");
    }
    if ((!params[0]->isNumber())||(!params[0]->isNumber())) {
        throw LispError("Cannot compare non-numeric values.");
    }
    double leftvalue = params[0]->asNumber();
    double rightvalue = params[1]->asNumber();
    return leftvalue <= rightvalue ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_is_even(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function is_even.");
    }
    if (params.size()!=1) {
        throw LispError("Only 1 value can be judged at a time.");
    }
    if ((!params[0]->isNumber())) {
        throw LispError("Cannot judge non-numeric values.");
    }
    double result = params[0]->asNumber();
    if (!(result==static_cast<int>(result))) {
        throw LispError("Only integers can be judged at a time.");
    } 
    return static_cast<int>(result) % 2 == 0 ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_is_odd(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function is_odd.");
    }
    if (params.size()!=1) {
        throw LispError("Only 1 value can be judged at a time.");
    }
    if ((!params[0]->isNumber())) {
        throw LispError("Cannot judge non-numeric values.");
    }
    double result = params[0]->asNumber();
    if (!(result==static_cast<int>(result))) {
        throw LispError("Only integers can be judged at a time.");
    } 
    return static_cast<int>(result) % 2 != 0 ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

static ValuePtr builtin_is_zero(const std::vector<ValuePtr>& params) {
    if (params.empty()) {
        throw LispError("Must have a value for function is_zero.");
    }
    if (params.size()!=1) {
        throw LispError("Only 1 value can be judged at a time.");
    }
    if ((!params[0]->isNumber())) {
        throw LispError("Cannot judge non-numeric values.");
    }
    double result = params[0]->asNumber();
    return result == 0 ? std::make_shared<BooleanValue>(true) : std::make_shared<BooleanValue>(false);
}

const BuiltinProceduresMap& get_builtin_procedures() {
    static BuiltinProceduresMap procedures_map;
    static bool initialized = false; 
    if (!initialized) {
        procedures_map["display"] = std::make_shared<BuiltinProcValue>(&builtin_display);
        procedures_map["exit"] = std::make_shared<BuiltinProcValue>(&builtin_exit);
        procedures_map["newline"] = std::make_shared<BuiltinProcValue>(&builtin_newline);
        procedures_map["print"] = std::make_shared<BuiltinProcValue>(&builtin_print);
        procedures_map["atom?"] = std::make_shared<BuiltinProcValue>(&builtin_atom);
        procedures_map["boolean?"] = std::make_shared<BuiltinProcValue>(&builtin_boolean);
        procedures_map["integer?"] = std::make_shared<BuiltinProcValue>(&builtin_integer);
        procedures_map["list?"] = std::make_shared<BuiltinProcValue>(&builtin_list_);
        procedures_map["number?"] = std::make_shared<BuiltinProcValue>(&builtin_number);
        procedures_map["null?"] = std::make_shared<BuiltinProcValue>(&builtin_null);
        procedures_map["pair?"] = std::make_shared<BuiltinProcValue>(&builtin_pair);
        procedures_map["procedure?"] = std::make_shared<BuiltinProcValue>(&builtin_procedure);
        procedures_map["string?"] = std::make_shared<BuiltinProcValue>(&builtin_string);
        procedures_map["symbol?"] = std::make_shared<BuiltinProcValue>(&builtin_symbol);
        procedures_map["append"] = std::make_shared<BuiltinProcValue>(&builtin_append);
        procedures_map["car"] = std::make_shared<BuiltinProcValue>(&builtin_car);
        procedures_map["cdr"] = std::make_shared<BuiltinProcValue>(&builtin_cdr);
        procedures_map["cons"] = std::make_shared<BuiltinProcValue>(&builtin_cons);
        procedures_map["length"] = std::make_shared<BuiltinProcValue>(&builtin_length);
        procedures_map["list"] = std::make_shared<BuiltinProcValue>(&builtin_list);
        procedures_map["+"] = std::make_shared<BuiltinProcValue>(&builtin_add);
        procedures_map["-"] = std::make_shared<BuiltinProcValue>(&builtin_subtract);
        procedures_map["*"] = std::make_shared<BuiltinProcValue>(&builtin_multiply);
        procedures_map["/"] = std::make_shared<BuiltinProcValue>(&builtin_divide);
        procedures_map["abs"] = std::make_shared<BuiltinProcValue>(&builtin_abs);
        procedures_map["expr"] = std::make_shared<BuiltinProcValue>(&builtin_expr);
        procedures_map["quotient"] = std::make_shared<BuiltinProcValue>(&builtin_quotient);
        procedures_map["modulo"] = std::make_shared<BuiltinProcValue>(&builtin_modulo);
        procedures_map["remainder"] = std::make_shared<BuiltinProcValue>(&builtin_remainder);
        procedures_map[">"] = std::make_shared<BuiltinProcValue>(&builtin_greater);
        procedures_map["<"] = std::make_shared<BuiltinProcValue>(&builtin_lesser);
        procedures_map["="] = std::make_shared<BuiltinProcValue>(&builtin_equal);
        procedures_map[">="] = std::make_shared<BuiltinProcValue>(&builtin_greater_equal);
        procedures_map["<="] = std::make_shared<BuiltinProcValue>(&builtin_lesser_equal);
        procedures_map["even?"] = std::make_shared<BuiltinProcValue>(&builtin_is_even);
        procedures_map["odd?"] = std::make_shared<BuiltinProcValue>(&builtin_is_odd);
        procedures_map["zero?"] = std::make_shared<BuiltinProcValue>(&builtin_is_zero);
        initialized = true;
    }
    return procedures_map;
}


