#include "eval_env.h"
#include "error.h"

#include <algorithm>
#include <iterator>

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
        std::vector<ValuePtr> elements_vec;
        try {
            elements_vec = expr->toVector();
        } catch (const std::runtime_error& e) {
            throw LispError("Invaild List");
        }
        if (elements_vec.empty()) {
            throw LispError("Attempt to evaluate a NilValue.");
        }
        ValuePtr op_expr = elements_vec[0];
        if (auto op_name_opt = op_expr->asSymbol()) { 
            const std::string& op_name = *op_name_opt;
            if (op_name == "quote") { 
                if (elements_vec.size() != 2) {
                    throw LispError("Invalid Quote");
                }
                return elements_vec[1];
            }
            if (op_name == "define") {
                if (elements_vec.size() != 3) {
                    throw LispError("Invalid Defination");
                }
                if (auto var_name_opt = elements_vec[1]->asSymbol()) {
                    const std::string& variable_name = *var_name_opt;
                    ValuePtr value_to_be_evaluated = elements_vec[2];
                    ValuePtr evaluated_value = this->eval(value_to_be_evaluated);
                    symbol_map[variable_name] = evaluated_value; 
                    return std::make_shared<NilValue>(); 
                } 
                else {
                    throw LispError("Invalid Defination");
                }
            }
            //Other Special Symbols...
        
        }
        ValuePtr proc_object = this->eval(op_expr);
        ValuePtr cdr_part_of_expr = std::make_shared<NilValue>();
        if (expr->isPair()){
            PairValue* original_pair = static_cast<PairValue*>(expr.get());
            if(original_pair) {
                cdr_part_of_expr = original_pair->r;
            } 
            else {
                throw LispError(" expr->isPair() is true but static_cast to PairValue failed.");
            }
        } 
        else {
            throw LispError("PairValue expected.");
        }
        std::vector<ValuePtr> evaluated_args = this->evalList(cdr_part_of_expr);
        return this->apply(proc_object, evaluated_args);
    }
    throw LispError("Cannot evaluate unexpected value type: " + expr->toString());
}

EvalEnv::EvalEnv() {
    for(auto i : get_builtin_procedures()){
        symbol_map[i.first]=i.second;
    }
}

ValuePtr EvalEnv::apply(ValuePtr proc_object, std::vector<ValuePtr> args) {  
    if (typeid(*proc_object) == typeid(BuiltinProcValue)) {
        auto builtin_proc = std::static_pointer_cast<BuiltinProcValue>(proc_object);
        BuiltinFuncType* func_to_call = builtin_proc->get_function_pointer();
        if (func_to_call) {
            try {
                return func_to_call(args);
            } catch (const LispError& e) {
                throw;
            } catch (const std::exception& e) {
                throw;
            }
        } 
        else {
            throw LispError("BuiltinFuncType* is nullptr.");
        }
    }
    else {
        throw LispError("Unimplemented");
    }
}


std::vector<ValuePtr> EvalEnv::evalList(ValuePtr expr_containing_args) {
    if (expr_containing_args->isNil()) {
        return {};
    }
    if (!expr_containing_args->isPair()) {
        throw LispError("Invaalid function evalList");
    }
    std::vector<ValuePtr> result;
    std::vector<ValuePtr> unevaluated_arg_expressions;
    try {
        unevaluated_arg_expressions = expr_containing_args->toVector();
    } catch (const std::runtime_error& e) {
        throw LispError("Invaalid function evalList");
    }
    for (const ValuePtr& expr_to_eval : unevaluated_arg_expressions) {
        result.push_back(this->eval(expr_to_eval));
    }
    return result;
}
