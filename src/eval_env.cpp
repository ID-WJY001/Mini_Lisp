#include "eval_env.h"
#include "error.h"

#include <algorithm>
#include <iterator>
#include <memory>

using namespace std::literals;

const ValuePtr LISP_NIL = std::make_shared<NilValue>();
const ValuePtr LISP_TRUE = std::make_shared<BooleanValue>(1);
const ValuePtr LISP_FALSE = std::make_shared<BooleanValue>(0);

std::unordered_map<std::string, ValuePtr> global_symbol_table;
ValuePtr EvalEnv::expandQuasiquote(const ValuePtr& tmpl) {
    if (!tmpl->isPair()) {
        return tmpl;
    }
    if (tmpl->isNil()) {
        return tmpl;
    }
    auto car = std::static_pointer_cast<PairValue>(tmpl)->l;
    if (car->isSymbol() && car->asSymbol() && *car->asSymbol() == "unquote") {
        auto cdr = std::static_pointer_cast<PairValue>(tmpl)->r;
        if (!cdr->isPair() || std::static_pointer_cast<PairValue>(cdr)->r->isNil() == false) {
            throw LispError("unquote: expects exactly one argument");
        }
        auto expr_to_eval = std::static_pointer_cast<PairValue>(cdr)->l;
        return this->eval(expr_to_eval);
    }
    
    auto expanded_car = this->expandQuasiquote(car);
    auto expanded_cdr = this->expandQuasiquote(std::static_pointer_cast<PairValue>(tmpl)->r);
    return std::make_shared<PairValue>(expanded_car, expanded_cdr);
}
ValuePtr create_or_get_symbol(const std::string& name) {
    auto it = global_symbol_table.find(name);
    if (it != global_symbol_table.end()) {
        return it->second;
    } else {
        ValuePtr new_symbol = std::make_shared<SymbolValue>(name);
        global_symbol_table[name] = new_symbol;
        return new_symbol;
    }
}

ValuePtr EvalEnv::eval(const ValuePtr &expr) {
    if (expr->isSymbol()) {
        auto name_opt = expr->asSymbol();
        return this->lookupBinding(*name_opt);
    }
    if (expr->isNumber() || expr->isString() || expr->isBoolean()) {
        return expr;
    }
    if (expr->isNil()) {
        return expr;
    }
    if (expr->isProcedure()) {
        return expr;
    }
    if (expr->isPair()) {
        std::vector<ValuePtr> elements_vec = expr->toVector();
        if (elements_vec.empty()) {
            throw LispError("Attempt to evaluate an empty application form.");
        }
        ValuePtr op_expr = elements_vec[0];
        if (op_expr->isSymbol()) {
            auto op_name_opt = op_expr->asSymbol();
            const std::string& op_name = *op_name_opt;
            auto it_sf = SPECIAL_FORMS.find(op_name);
            if (it_sf != SPECIAL_FORMS.end()) {
                SpecialFormType* form_func = it_sf->second; 
                std::vector<ValuePtr> form_args;
                if (elements_vec.size() > 1) {
                    form_args.assign(elements_vec.begin() + 1, elements_vec.end());
                }
                return form_func(form_args, *this);
            }
        }
        ValuePtr proc_object = this->eval(op_expr);
        if (auto macro = std::dynamic_pointer_cast<MacroValue>(proc_object)) {
            ValuePtr args_expr = std::static_pointer_cast<PairValue>(expr)->r;
            std::vector<ValuePtr> arg_values;
            if (!args_expr->isNil()) {
                arg_values = args_expr->toVector();
            }
            if (macro->params.size() != arg_values.size()) {
                throw LispError("Macro argument count mismatch");
            }
            auto macro_env = std::make_shared<EvalEnv>(this->shared_from_this());
            for (size_t i = 0; i < macro->params.size(); ++i) {
                macro_env->defineBinding(macro->params[i], arg_values[i]);
            }
            auto expanded = macro_env->eval(macro->body);
            return this->eval(expanded);
        }
        if (!proc_object->isProcedure()) {
            throw LispError("Operator is not a procedure.");
        }
        ValuePtr args_expressions_list = std::static_pointer_cast<PairValue>(expr)->r;
        std::vector<ValuePtr> evaluated_args = this->evalList(args_expressions_list);
        return this->apply(proc_object, evaluated_args);
    }
    throw LispError("Cannot evaluate unexpected value type: " + expr->toString());
}

ValuePtr EvalEnv::apply(ValuePtr proc_object, std::vector<ValuePtr> args) {
    if (typeid(*proc_object) == typeid(BuiltinProcValue)) {
        auto builtin_proc = std::static_pointer_cast<BuiltinProcValue>(proc_object);
        BuiltinFuncType func_to_call = builtin_proc->get_function_pointer();
        if (func_to_call) {
            try {
                return func_to_call(args, *this);
            } catch (const LispError& e) {
                throw;
            } catch (const std::exception& e) {
                throw LispError("Exception in builtin procedure " + proc_object->toString() + ": " + e.what());
            }
        }
        else {
            throw LispError("BuiltinFuncType* is nullptr for " + proc_object->toString());
        }
    }
    else if (auto lambda_proc = std::dynamic_pointer_cast<LambdaValue>(proc_object)) {
        const auto& formal_params = lambda_proc->get_params();
        const auto& body_expressions = lambda_proc->get_body();
        std::shared_ptr<EvalEnv> captured_env = lambda_proc->get_captured_env();
        if (formal_params.size() != args.size()) {
            throw LispError("Eval::apply error.");
        }
        auto call_env = std::make_shared<EvalEnv>(captured_env);
        for (size_t i = 0; i < formal_params.size(); ++i) {
           call_env->defineBinding(formal_params[i], args[i]);
        }
        ValuePtr result = LISP_NIL; 
        for (const auto& body_expr : body_expressions) {
           result = call_env->eval(body_expr);
        }
        return result;
    }
    else {
        throw LispError("Unimplemented: Cannot apply non-builtin procedure: " + proc_object->toString());
    }
}

std::vector<ValuePtr> EvalEnv::evalList(ValuePtr expr_containing_args) {
    if (expr_containing_args->isNil()) {
        return {};
    }
    if (!expr_containing_args->isPair()) {
        throw LispError("Invalid argument list structure for evalList: expected a proper list, got " + expr_containing_args->toString()); 
    }

    std::vector<ValuePtr> result;
    std::vector<ValuePtr> unevaluated_arg_expressions;
    try {
        unevaluated_arg_expressions = expr_containing_args->toVector();
    } catch (const std::runtime_error& e) {
        throw LispError("Invalid argument list structure during toVector in evalList: "s + e.what()); 
    }

    for (ValuePtr& expr_to_eval : unevaluated_arg_expressions) { 
        result.push_back(this->eval(expr_to_eval));
    }
    return result;
}

EvalEnv::EvalEnv() : parent(nullptr) {
    for(auto const& pair : get_builtin_procedures()){ 
        this->defineBinding(pair.first, pair.second);
    }
}

ValuePtr EvalEnv::lookupBinding(const std::string& name) {
    std::shared_ptr<EvalEnv> current_env = shared_from_this();
    while (current_env) {
        auto it = current_env->symbol_map.find(name);
        if (it != current_env->symbol_map.end()) {
            return it->second;
        }
        current_env = current_env->parent; 
    }
    throw LispError("Variable " + name + " not defined.");
}

void EvalEnv::defineBinding(const std::string& name, ValuePtr value) {
    symbol_map[name] = value;
}