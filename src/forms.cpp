#include "forms.h"
#include "error.h"
#include "eval_env.h"
#include "value.h"
#include <iostream>

std::vector<std::string> get_parameter_names(ValuePtr param_list_node) {
    std::vector<std::string> names;
    ValuePtr current = param_list_node;
    while (current && !current->isNil()) { // 假设 is_nil() 检查 Lisp nil
        auto pair_node = std::dynamic_pointer_cast<PairValue>(current);
        if (!pair_node) {
            throw LispError("Invalid function definition: parameter list is not a proper list.");
        }
        ValuePtr param_symbol_ptr = pair_node->l; // 或者 pair_node->l
        if (!param_symbol_ptr->isSymbol()) {
            throw LispError("Invalid function definition: parameters must be symbols.");
        }
        auto symbol_name_opt = param_symbol_ptr->asSymbol(); // 假设 asSymbol() 返回 std::optional<std::string>
        if (!symbol_name_opt) { // 额外的检查，理论上 isSymbol() 通过了这里应该也通过
            throw LispError("Internal error: parameter symbol has no name.");
        }
        names.push_back(*symbol_name_opt);
        current = pair_node->r; // 或者 pair_node->r
    }
    if (current && !current->isNil()) {
        throw LispError("Invalid function definition: parameter list is an improper list.");
    }
    return names;
}

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() < 2) { 
        throw LispError("Invalid Definition: `define` requires at least a name and a value/body.");
    }
    if (args[0]->isPair()) { // 函数定义 (define (f params...) body...)
        if (args.size() < 2) {
             throw LispError("Invalid function definition: `define` for function needs name/params and at least one body expression.");
        }
        ValuePtr func_spec_pair = args[0]; 
        auto pair_spec = std::dynamic_pointer_cast<PairValue>(func_spec_pair);
        if (!pair_spec) {
            throw LispError("Internal error: expected a pair for function specification.");
        }
        ValuePtr func_name_val = pair_spec->l;
        if (!func_name_val->isSymbol()) {
            throw LispError("Invalid function definition: function name must be a symbol.");
        }
        auto func_name_opt = func_name_val->asSymbol();
        if (!func_name_opt) {
            throw LispError("Internal error: function symbol has no name.");
        }
        const std::string& function_name_str = *func_name_opt; // 简化获取
        std::vector<std::string> param_names_vec = get_parameter_names(pair_spec->r);
        std::vector<ValuePtr> body_expressions;
        if (args.size() < 2) { 
             throw LispError("Function definition requires a name/parameters and at least one body expression.");
        }
        body_expressions.assign(args.begin() + 1, args.end());
            if (body_expressions.empty()){
            throw LispError("Function body cannot be empty.");
        }
        auto lambda_val = std::make_shared<LambdaValue>(function_name_str, param_names_vec, body_expressions, env.shared_from_this());
        env.defineBinding(function_name_str, lambda_val); 
        return std::make_shared<NilValue>();
    }
    // ... (变量定义部分)
    else if (args[0]->isSymbol()) {
        if (args.size() != 2) {
            throw LispError("Invalid variable definition: `define` for variable needs a name and exactly one value.");
        }
        auto var_name_opt = args[0]->asSymbol();
        if (!var_name_opt) {
            throw LispError("Internal error: variable symbol has no name.");
        }
        const std::string& variable_name = *var_name_opt;
        ValuePtr value_to_be_evaluated = args[1];
        ValuePtr evaluated_value = env.eval(value_to_be_evaluated);
        env.defineBinding(variable_name, evaluated_value); // 使用 defineBinding
        return std::make_shared<NilValue>();
    }
    else {
        throw LispError("Invalid Definition: first argument to `define` must be a symbol (for variable) or a list (for function). Actual: " + args[0]->toString());
    }
}

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1){
        throw LispError("Invalid Quote: quote needs only one value.");
    }
    return args[0];
}

ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 3) {
        throw LispError("Invalid Definition: if needs only 3 values.");
    }
    ValuePtr condition_expression = args[0];
    ValuePtr condition_result = env.eval(condition_expression);
    if (!condition_result->isLispFalse()) { // 如果条件不是 Lisp false (即是 Lisp true)
        ValuePtr branch = args[1];
        return env.eval(branch);
    } 
    else { // 如果条件是 Lisp false
        ValuePtr branch = args[2];
        return env.eval(branch);
    }
}

ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        return std::make_shared<BooleanValue>(true); 
    }
    ValuePtr last_eval_result = nullptr; 
    for (const auto& expr : args) { 
        last_eval_result = env.eval(expr);
        if (last_eval_result->isLispFalse()) {
            return std::make_shared<BooleanValue>(false);
        }
    }
    return last_eval_result;
}

ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for(auto i : args){
        ValuePtr branch = i;
        auto judge_env = env.eval(branch);
        auto judge = std::make_shared<BooleanValue>(!judge_env->isLispFalse());
        if(judge->getboolValue()){
            return judge_env;
        }
    }
    return std::make_shared<BooleanValue>(0);
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if(args.size() != 2){
        throw LispError("Invalid lambda definition.");
    }
    ValuePtr params_list_node = args[0];
    ValuePtr body_node = args[1];
    std::vector<std::string> param_names_vec = get_parameter_names(params_list_node);
    std::vector<ValuePtr> body_expressions;
    if (args.size() < 2) {
         throw LispError("Lambda form requires parameters and at least one body expression.");
    }
    body_expressions.assign(args.begin() + 1, args.end());
    if (body_expressions.empty()){
        throw LispError("Lambda body cannot be empty.");
    }
    return std::make_shared<LambdaValue>("<lambda>", param_names_vec, body_expressions, env.shared_from_this());
}

const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"define", defineForm}, 
    {"quote",  quoteForm},
    {"if", ifForm},
    {"and", andForm},
    {"or", orForm},
    {"lambda",lambdaForm}
};