#include "eval_env.h"
#include "error.h"

using namespace std::literals;

ValuePtr EvalEnv::eval(ValuePtr expr) {
    if (auto name_opt = expr->asSymbol()) {
        const std::string& name = *name_opt; 
        auto it = symbol_map.find(name);
        if (it != symbol_map.end()) {
            return it->second;
        } 
        else {
            throw LispError("Variable " + name + " not defined.");
        }
    }
    if ((*expr).isSelfEvaluating()) {
        return expr;
    } 
    if ((*expr).isNil()) {
        throw LispError("Evaluating nil is prohibited.");
    }
    if (expr->isPair()) {
        std::vector<ValuePtr> v;
        try {
            v = expr->toVector(); 
        } catch (const std::runtime_error& e) {
            throw LispError("Invalid list structure: " + expr->toString() + " - " + e.what());
        }
        ValuePtr op = v[0];
        if (op->asSymbol() == "define"s) {
            if (v.size() != 3) {
                throw LispError("Syntax error (define): expects (define name value). Got " +
                                std::to_string(v.size() -1) + " arguments in: " + expr->toString());
            }
            if (auto name_opt = v[1]->asSymbol()) {
                const std::string& variable_name = *name_opt;
                ValuePtr value_expression = v[2];
                ValuePtr evaluated_value = this->eval(value_expression);
                symbol_map[variable_name] = evaluated_value;
                return std::make_shared<NilValue>();
            } 
            else {
                throw LispError("Syntax error (define): variable name must be a symbol. Got: " + v[1]->toString());
            }
        }
        else {
            throw LispError("Unimplemented procedure call or unknown special form: " + op->toString());
        }
    }
    throw LispError("Cannot evaluate unexpected value type: " + expr->toString());
}