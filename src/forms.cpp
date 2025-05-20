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
    if (args[0]->isPair()) {
        if (args.size() < 2) {
             throw LispError("Invalid function definition: `define` for function needs name/params and at least one body expression.");
        }
        auto func_spec_pair = std::dynamic_pointer_cast<PairValue>(args[0]);
        if (!func_spec_pair) {
            throw LispError("Internal error: expected a pair for function specification.");
        }
        ValuePtr func_name_val = func_spec_pair->l;
        if (!func_name_val->isSymbol()) {
            throw LispError("Invalid function definition: function name must be a symbol.");
        }
        auto func_name_opt = func_name_val->asSymbol();
        if (!func_name_opt) {
            throw LispError("Internal error: function symbol has no name.");
        }
        const std::string& function_name_str = *func_name_opt;
        ValuePtr lisp_param_list = func_spec_pair->r;
        std::vector<std::string> param_names_vec = get_parameter_names(lisp_param_list);
        std::vector<ValuePtr> body_expressions;
        for (size_t i = 1; i < args.size(); ++i) {
            body_expressions.push_back(args[i]);
        }
        // 如果你的 Lisp 规定 (define (f p) (begin body1 body2 ...))
        // 那么函数体就是 args[1] 本身，它应该是一个列表 (begin ...)，或者 LambdaValue 需要处理单个表达式作为函数体的情况
        // 从你的 `args[1]->toVector()` 来看，你可能期望 `args[1]` 是一个包含所有body的列表。
        // 如果是这样，并且 LambdaValue 的第二个参数期望的是一个 `std::vector<ValuePtr>`
        // 代表多个顶级表达式，那么上面的 for 循环是正确的。
        // 如果 `LambdaValue` 的 body 就是一个单一的 `ValuePtr` (可能是 `begin` 表达式)，
        // 那么你需要调整。
        // 目前我假设 `LambdaValue` 的第二个参数 `std::vector<ValuePtr> body` 指的是顶级函数体表达式的列表。
        env.symbol_map[function_name_str] = std::make_shared<LambdaValue>(function_name_str, param_names_vec, body_expressions);
        return std::make_shared<NilValue>();
    }
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
        env.symbol_map[variable_name] = evaluated_value;
        return std::make_shared<NilValue>();
    }
    else {
        throw LispError("Invalid Definition: first argument to `define` must be a symbol or a list for function definition.");
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
    if(!args[0]->isPair()){
        throw LispError("Invalid lambda definition.");
    }
    if(!args[1]->isPair()){
        throw LispError("Invalid lambda definition.");
    }
    std::vector<std::string> param_names_vec = get_parameter_names(args[0]);
    return std::make_shared<LambdaValue>("<lambda>", param_names_vec, args[1]->toVector());
}

const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"define", defineForm}, 
    {"quote",  quoteForm},
    {"if", ifForm},
    {"and", andForm},
    {"or", orForm},
    {"lambda",lambdaForm}
};