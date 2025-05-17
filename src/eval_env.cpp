#include "eval_env.h"
#include "error.h"

#include <algorithm>
#include <iterator>

using namespace std::literals;

ValuePtr EvalEnv::eval(ValuePtr &expr) {
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
                // 这是一个特殊形式!
                SpecialFormType* form_func = it_sf->second; // 获取处理函数指针

                // 准备传递给特殊形式处理函数的参数。
                // 这些是操作符之后的元素 (cdr 部分)。
                std::vector<ValuePtr> form_args;
                if (elements_vec.size() > 1) {
                    // assign 方法: 将 elements_vec 中从第二个元素到末尾的元素复制到 form_args
                    form_args.assign(elements_vec.begin() + 1, elements_vec.end());
                }
                // else form_args 保持为空 (例如，对于一个假设的无参数特殊形式)

                // 调用特殊形式的处理函数，传递未求值的参数和环境
                return form_func(form_args, *this);
            }

            // --- 原来硬编码的 'define' 和 'quote' 逻辑块从这里移除 ---
            // if (op_name == "quote") { /* ... */ } // 已移除
            // if (op_name == "define") { /* ... */ } // 已移除
        }

        // 如果不是特殊形式 (或者操作符不是符号)，则为过程调用。
        ValuePtr proc_object = this->eval(op_expr); // 对操作符部分进行求值

        // 获取参数列表 (原始表达式的 cdr 部分)
        ValuePtr cdr_part_of_expr = std::make_shared<NilValue>(); // 默认为空参数列表
        // 以下获取 cdr_part_of_expr 的逻辑来自您提供的原始代码。
        // 它有点防御性/冗余，因为我们已经在一个 `if (expr->isPair())` 块内。
        if (expr->isPair()){ // expr 是原始的完整表达式，例如 (f arg1 arg2)
            PairValue* original_pair = static_cast<PairValue*>(expr.get());
            if(original_pair) { // 如果 expr->isPair() 为真，此检查有点多余
                cdr_part_of_expr = original_pair->r; // 这是 (arg1 arg2 ...) 形式的 ValuePtr (列表)
            }
            else {
                // 这意味着 expr->isPair() 为真，但 static_cast 失败了。
                // 表明存在严重的内部不一致。
                throw LispError("Internal error: expr->isPair() is true but static_cast to PairValue failed.");
            }
        }
        else {
            // 如果外层的 `if (expr->isPair())` 为真，这个 'else' 分支应该不可达。
            // 保留自原始结构，以最小化任务范围之外的更改。
            throw LispError("Internal error: PairValue expected for procedure call arguments preparation.");
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
