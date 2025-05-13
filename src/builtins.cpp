#include <iostream>
#include "builtins.h"

ValuePtr add(const std::vector<ValuePtr>& params) {
    auto result = 0.0;
    for (const auto& i : params) {
        if (!i->isNumber()) {
            throw LispError("Cannot add a non-numeric value.");
        }
        result += i->asNumber();
    }
    return std::make_shared<NumericValue>(result);
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

const BuiltinProceduresMap& get_builtin_procedures() {
    static BuiltinProceduresMap procedures_map;
    static bool initialized = false; 

    if (!initialized) {
        procedures_map["+"] = std::make_shared<BuiltinProcValue>(&builtin_add);
        procedures_map["-"] = std::make_shared<BuiltinProcValue>(&builtin_subtract);
        procedures_map["*"] = std::make_shared<BuiltinProcValue>(&builtin_multiply);
        procedures_map["/"] = std::make_shared<BuiltinProcValue>(&builtin_divide);
        procedures_map["print"] = std::make_shared<BuiltinProcValue>(&builtin_print);
        initialized = true;
    }
    return procedures_map;
}


