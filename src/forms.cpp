#include "forms.h"
#include "error.h"
#include "eval_env.h"
#include "value.h"
#include <iostream>

std::vector<std::string> get_parameter_names(ValuePtr param_list_node) {
    std::vector<std::string> names;
    ValuePtr current = param_list_node;
    while (current && !current->isNil()) {
        auto pair_node = std::dynamic_pointer_cast<PairValue>(current);
        if (!pair_node) {
            throw LispError("Invalid function definition: parameter list is not a proper list.");
        }
        ValuePtr param_symbol_ptr = pair_node->l;
        if (!param_symbol_ptr->isSymbol()) {
            throw LispError("Invalid function definition: parameters must be symbols.");
        }
        auto symbol_name_opt = param_symbol_ptr->asSymbol(); 
        if (!symbol_name_opt) { 
            throw LispError("Internal error: parameter symbol has no name.");
        }
        names.push_back(*symbol_name_opt);
        current = pair_node->r;
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
        const std::string& function_name_str = *func_name_opt;
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
        return LISP_NIL;
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
        env.defineBinding(variable_name, evaluated_value);
        return LISP_NIL;
    }
    else {
        throw LispError("Invalid Definition: first argument to `define` must be a symbol (for variable) or a list (for function). Actual: " + args[0]->toString());
    }
}

ValuePtr condForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()){
        throw LispError("Invalid Cond.");
    }
    for(auto it : args){
        auto it_vector = it->toVector();
        if (it_vector.empty()){
            throw LispError("Invalid Cond.");
        }
        if(it_vector[0]->toString() == "else" && it != args.back()){
            throw LispError("Invalid Cond : else error.");
        }
        if(it_vector.size() == 1){
            return env.eval(it_vector[0]);
        }
        if(it_vector[0]->toString() == "else" && it == args.back()){
            return env.eval(it_vector.back());
        }
        if(!env.eval(it_vector[0])->isLispFalse()){
            for(auto i : it_vector){
                env.eval(i);
            }
            return env.eval(it_vector.back());
        }
        else{
            continue;
        }
    }
    return args[0];
}

ValuePtr beginForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()){
        throw LispError("Invalid Begin.");
    }
    for(auto it : args){
        env.eval(it);
    }
    return env.eval(args.back());
}

ValuePtr letForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() < 2) {
        throw LispError("let: requires bindings and at least one body expression.");
    }
    ValuePtr bindings_node = args[0];
    if (!bindings_node->isList() && !bindings_node->isNil()) {
        throw LispError("let: bindings must be a list. Got: " + bindings_node->toString());
    }
    auto let_env = std::make_shared<EvalEnv>(env.shared_from_this());
    std::vector<std::pair<std::string, ValuePtr>> evaluated_bindings_for_let_env;
    if (!bindings_node->isNil()) { 
        std::vector<ValuePtr> binding_pairs_vec = bindings_node->toVector();
        for (const auto& binding_pair_node : binding_pairs_vec) {
            if (!binding_pair_node->isList()) { 
                throw LispError("let: each binding must be a list (variable expression). Got: " + binding_pair_node->toString());
            }
            std::vector<ValuePtr> pair_vec = binding_pair_node->toVector();
            if (pair_vec.size() != 2) {
                throw LispError("let: each binding must be a pair (variable expression) of size 2. Got: " + binding_pair_node->toString());
            }
            ValuePtr var_symbol_ptr = pair_vec[0];
            ValuePtr val_expr_ptr = pair_vec[1];
            if (!var_symbol_ptr->isSymbol()) {
                throw LispError("let: variable name in binding must be a symbol. Got: " + var_symbol_ptr->toString());
            }
            auto var_name_opt = var_symbol_ptr->asSymbol();
            ValuePtr evaluated_val = env.eval(val_expr_ptr);
            evaluated_bindings_for_let_env.push_back({*var_name_opt, evaluated_val});
        }
    }
    for (const auto& eb : evaluated_bindings_for_let_env) {
        let_env->defineBinding(eb.first, eb.second);
    }
    ValuePtr result = LISP_NIL; 
    for (size_t i = 1; i < args.size(); ++i) { 
        result = let_env->eval(args[i]);
    }
    return result;
}

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1){
        throw LispError("Invalid Quote: quote needs only one value.");
    }
    return args[0];
}

ValuePtr quasiquoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1) {
        throw LispError("quasiquote: expects exactly one argument");
    }
    return env.expandQuasiquote(args[0]);
}

ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 2 && args.size() != 3) {
        throw LispError("if: bad syntax. Expected (if condition then-expr [else-expr])");
    }
    
    ValuePtr condition_expression = args[0];
    ValuePtr condition_result = env.eval(condition_expression);
    if (!condition_result->isLispFalse()) {
        ValuePtr then_branch = args[1];
        return env.eval(then_branch);
    } else {
        if (args.size() == 3) {
            ValuePtr else_branch = args[2];
            return env.eval(else_branch);
        } else {
            return std::make_shared<NilValue>(); 
        }
    }
}

ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.empty()) {
        return LISP_TRUE; 
    }
    ValuePtr last_eval_result = nullptr; 
    for (const auto& expr : args) { 
        last_eval_result = env.eval(expr);
        if (last_eval_result->isLispFalse()) {
            return LISP_FALSE;
        }
    }
    return last_eval_result;
}

ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for(auto i : args){
        ValuePtr branch = i;
        auto judge_env = env.eval(branch);
        auto judge = (!judge_env->isLispFalse())?LISP_TRUE:LISP_FALSE;
        if(judge->getboolValue()){
            return judge_env;
        }
    }
    return LISP_FALSE;
}

ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if(args.size() < 2){
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

ValuePtr defineMacroForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() < 3) {
        throw LispError("define-macro: expects (name (params) body)");
    }
    // 解析名字
    auto name_sym = std::dynamic_pointer_cast<SymbolValue>(args[0]);
    if (!name_sym) throw LispError("define-macro: first argument must be symbol");
    // 解析参数
    std::vector<std::string> param_names = get_parameter_names(args[1]);
    // 宏体
    ValuePtr body = args[2];
    auto macro = std::make_shared<MacroValue>(param_names, body);
    env.defineBinding(name_sym->toString(), macro);
    return macro;
}

const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"cond", condForm},
    {"begin", beginForm},
    {"let", letForm},
    {"define", defineForm}, 
    {"quote",  quoteForm},
    {"quasiquote",  quasiquoteForm},
    {"if", ifForm},
    {"and", andForm},
    {"or", orForm},
    {"lambda",lambdaForm},
    {"define-macro", defineMacroForm}
};