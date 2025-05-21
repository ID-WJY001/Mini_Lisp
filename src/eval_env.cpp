#include "eval_env.h"
#include "error.h"

#include <algorithm>
#include <iterator>
#include <memory>

using namespace std::literals;

ValuePtr EvalEnv::eval(const ValuePtr &expr) {
    if (auto name_opt = expr->asSymbol()) {
        const std::string& name = *name_opt;
        return this->lookupBinding(name);
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
            // 例如，对于 (define x 10)，elements_vec 是 [ValuePtr(Symbol("define")), ValuePtr(Symbol("x")), ValuePtr(Number(10))]
            // 对于 (+ 1 2)，elements_vec 是 [ValuePtr(Symbol("+")), ValuePtr(Number(1)), ValuePtr(Number(2))]
            elements_vec = expr->toVector();
        } catch (const std::runtime_error& e) {
            throw LispError("Invalid List structure for evaluation: "s + e.what()); // "Invaild" -> "Invalid"
        }
        if (elements_vec.empty()) {
            // 这个情况理论上不应通过 isPair() 为真且 toVector() 为空到达。
            // 原始代码错误信息："Attempt to evaluate a NilValue."
            // 如果到达此状态，使用更通用的消息。
            throw LispError("Attempt to evaluate an empty application form.");
        }
        ValuePtr op_expr = elements_vec[0]; // 操作符或特殊形式的名称
        // 检查操作符是否为符号，并且是否为特殊形式
        if (auto op_name_opt = op_expr->asSymbol()) {
            const std::string& op_name = *op_name_opt;
            auto it_sf = SPECIAL_FORMS.find(op_name); // it_sf 表示 "iterator special form"
            if (it_sf != SPECIAL_FORMS.end()) {
                SpecialFormType* form_func = it_sf->second; // 获取处理函数指针
                std::vector<ValuePtr> form_args;
                if (elements_vec.size() > 1) {
                    form_args.assign(elements_vec.begin() + 1, elements_vec.end());
                }
                // 它应该使用 std::make_shared<EvalEnv>(this->shared_from_this())
                // 或 std::make_shared<EvalEnv>(env.get_shared_this()) 来创建子环境。
                return form_func(form_args, *this);
            }
        }
        ValuePtr proc_object = this->eval(op_expr);
        ValuePtr cdr_part_of_expr = std::make_shared<NilValue>();
        if (expr->isPair()){
            PairValue* original_pair = static_cast<PairValue*>(expr.get());
            if(original_pair) {
                cdr_part_of_expr = original_pair->r; 
            }
            else {
                throw LispError("Internal error: expr->isPair() is true but static_cast to PairValue failed.");
            }
        }
        else {
            throw LispError("Internal error: PairValue expected for procedure call arguments preparation.");
        }

        std::vector<ValuePtr> evaluated_args = this->evalList(cdr_part_of_expr);
        return this->apply(proc_object, evaluated_args);
    }

    throw LispError("Cannot evaluate unexpected value type: " + expr->toString());
}

ValuePtr EvalEnv::apply(ValuePtr proc_object, std::vector<ValuePtr> args) {
    // 处理 BuiltinProcValue
    if (typeid(*proc_object) == typeid(BuiltinProcValue)) { // 或者用 dynamic_pointer_cast
        auto builtin_proc = std::static_pointer_cast<BuiltinProcValue>(proc_object);
        BuiltinFuncType* func_to_call = builtin_proc->get_function_pointer();
        if (func_to_call) {
            try {
                return func_to_call(args);
            } catch (const LispError& e) {
                throw;
            } catch (const std::exception& e) {
                // 最好包装成 LispError，或者至少记录信息
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
        ValuePtr result = std::make_shared<NilValue>(); // 默认
        for (const auto& body_expr : body_expressions) {
           result = call_env->eval(body_expr);
        }
        return result;
    }
    else { // 如果不是 BuiltinProcValue，并且没有处理 LambdaValue，就会执行到这里
        throw LispError("Unimplemented: Cannot apply non-builtin procedure: " + proc_object->toString());
        // 或者更具体点，如果 proc_object 是 LambdaValue 但你还没写处理它的代码：
        // if (std::dynamic_pointer_cast<LambdaValue>(proc_object)) {
        //     throw LispError("Unimplemented: Lambda application not yet fully implemented.");
        // } else {
        //     throw LispError("Cannot apply non-procedure value: " + proc_object->toString());
        // }
    }
}

std::vector<ValuePtr> EvalEnv::evalList(ValuePtr expr_containing_args) {
    if (expr_containing_args->isNil()) {
        return {}; // 空的参数列表求值为空的值向量
    }
    // `evalList` 期望一个合法的参数列表。
    // 如果 `expr_containing_args` 不是 pair (且不是 Nil)，则它是一个不正确的参数列表。
    if (!expr_containing_args->isPair()) {
        throw LispError("Invalid argument list structure for evalList: expected a proper list, got " + expr_containing_args->toString()); // "Invaalid" -> "Invalid"
    }

    std::vector<ValuePtr> result;
    std::vector<ValuePtr> unevaluated_arg_expressions;
    try {
        // 这假设 expr_containing_args 是一个合法的列表。
        // toVector() 应该处理这个问题。
        unevaluated_arg_expressions = expr_containing_args->toVector();
    } catch (const std::runtime_error& e) {
        throw LispError("Invalid argument list structure during toVector in evalList: "s + e.what()); // "Invaalid" -> "Invalid"
    }

    for (ValuePtr& expr_to_eval : unevaluated_arg_expressions) { // 将 ValuePtr 按引用传递给 eval
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