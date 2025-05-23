#include <iostream>
#include <iomanip>
#include <cmath> 
#include <cstdlib>
#include "builtins.h"
#include "value.h"   
#include "eval_env.h"
#include "error.h" 


static ValuePtr builtin_apply(const std::vector<ValuePtr>& evaluated_args_for_apply_func, EvalEnv& env) {
    if(evaluated_args_for_apply_func.size() != 2){
        throw LispError("apply: Exactly 2 values required.");
    }
    ValuePtr proc_object_to_call = evaluated_args_for_apply_func[0]; 
    ValuePtr list_of_actual_args = evaluated_args_for_apply_func[1]; 
    if(!proc_object_to_call->isProcedure()){ 
        throw LispError("apply: Invalid variable.");
    }
    if(!list_of_actual_args->isList() && !list_of_actual_args->isNil()){
        throw LispError("apply: Invalid variable.");
    }
    std::vector<ValuePtr> args_vector_for_proc;
    if (!list_of_actual_args->isNil()) {
        args_vector_for_proc = list_of_actual_args->toVector();
    }

    return env.apply(proc_object_to_call, args_vector_for_proc);
}

static ValuePtr builtin_display(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.size() != 1){
        throw LispError("display: Exactly 1 value required.");
    }
    auto it = params[0];
    if(auto str_val = std::dynamic_pointer_cast<StringValue>(it)){ 
        std::cout << str_val->asString(); 
    }
    else{
        std::cout << it->toString();
    }
    return LISP_NIL;
}

static ValuePtr builtin_displayln(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.size() != 1){
        throw LispError("displayln: Exactly 1 value required.");
    }
    auto it = params[0];
    if(auto str_val = std::dynamic_pointer_cast<StringValue>(it)){ 
        std::cout << str_val->asString(); 
    }
    else{
        std::cout << it->toString();
    }
    std::cout << std::endl;
    return LISP_NIL;
}

static ValuePtr builtin_error(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.empty()){
        throw std::runtime_error(0);
    }
    if(params.size() != 1){
        throw LispError("error: At most 1 argument (error code) allowed.");
    }
    auto it = params[0];
    if(!it->isNumber()){ 
        throw LispError("error: Error code must be a number. Got: " + it->toString());
    }
    double val = it->asNumber();
    int error_code = static_cast<int>(val);
    if (std::abs(val - static_cast<double>(error_code)) > 1e-9) {
        throw LispError("error: Error code must be an integer. Got: " + it->toString());
    }
    std::string error_string = std::to_string(error_code); 
    throw std::runtime_error(error_string);
    return LISP_NIL;
}

static ValuePtr builtin_eval(const std::vector<ValuePtr>& params, EvalEnv& env) {
    if(params.size() != 1){
        throw LispError("eval: Exactly 1 value required.");
    }
    return env.eval(params[0]);
}

static ValuePtr builtin_exit(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.empty()){
        std::exit(0);
    }
    if(params.size() != 1){
        throw LispError("exit: At most 1 argument (exit code) allowed.");
    }
    auto it = params[0];
    if(!it->isNumber()){ // 确保是数字
        throw LispError("exit: Exit code must be a number. Got: " + it->toString());
    }
    double val = it->asNumber(); // 获取 double
    int exit_code = static_cast<int>(val);
    if (std::abs(val - static_cast<double>(exit_code)) > 1e-9) { // 检查是否为整数
        throw LispError("exit: Exit code must be an integer. Got: " + it->toString());
    }
    std::exit(exit_code);
    return LISP_NIL; // Unreachable
}

static ValuePtr builtin_newline(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(!params.empty()){
        throw LispError("newline: No arguments expected.");
    }
    std::cout << std::endl;
    return LISP_NIL;
}

static ValuePtr builtin_print(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
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
    return LISP_NIL;
}

static ValuePtr builtin_atom(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("atom?: expects 1 argument");
    ValuePtr p = params[0];
    return (p->isNil() || p->isBoolean() || p->isNumber() || p->isString() || p->isSymbol())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_boolean(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("boolean?: expects 1 argument");
    return (params[0]->isBoolean())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_integer(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("integer?: expects 1 argument");
    if (!params[0]->isNumber()) return LISP_FALSE;
    double val = params[0]->asNumber();
    return (std::trunc(val) == val)?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_list_(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("list?: expects 1 argument");
    return (params[0]->isList() || params[0]->isNil())?LISP_TRUE:LISP_FALSE; // 使用 Value::isList()
}
static ValuePtr builtin_number(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("number?: expects 1 argument");
    return (params[0]->isNumber())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_null(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("null?: expects 1 argument");
    return (params[0]->isNil())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_pair(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("pair?: expects 1 argument");
    return (params[0]->isPair())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_procedure(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("procedure?: expects 1 argument");
    return (std::dynamic_pointer_cast<BuiltinProcValue>(params[0]) || std::dynamic_pointer_cast<LambdaValue>(params[0]))?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_string(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("string?: expects 1 argument");
    return (params[0]->isString())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_symbol(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 1) throw LispError("symbol?: expects 1 argument");
    return (params[0]->isSymbol())?LISP_TRUE:LISP_FALSE;
}

static ValuePtr builtin_append(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.empty()) {
        return LISP_NIL;
    }
    ValuePtr result_head = LISP_NIL;
    ValuePtr current_new_tail = nullptr;

    for (size_t i = 0; i < params.size(); ++i) {
        ValuePtr current_list_part = params[i];
        if (i < params.size() - 1) {
            if (!current_list_part->isList() && !current_list_part->isNil()) {
                throw LispError("append: arguments before the last must be proper lists. Got: " + current_list_part->toString());
            }
            ValuePtr temp_iter = current_list_part;
            while (temp_iter->isPair()) {
                auto pair_node = std::static_pointer_cast<PairValue>(temp_iter);
                auto new_cell = std::make_shared<PairValue>(pair_node->l, LISP_NIL);
                if (result_head->isNil()) {
                    result_head = new_cell;
                } else {
                    std::static_pointer_cast<PairValue>(current_new_tail)->r = new_cell;
                }
                current_new_tail = new_cell;
                temp_iter = pair_node->r;
            }
        } 
        else { 
            if (result_head->isNil()) { 
                return current_list_part; 
            } 
            else {
                std::static_pointer_cast<PairValue>(current_new_tail)->r = current_list_part; 
            }
        }
    }
    return result_head;
}
static ValuePtr builtin_car(const std::vector<ValuePtr>& params, EvalEnv& env) {
    if(params.size() != 1) throw LispError("car: expects 1 argument");
    if(!params[0]->isPair()) throw LispError("car: argument must be a pair. Got: " + params[0]->toString());
    return std::static_pointer_cast<PairValue>(params[0])->l;
}
static ValuePtr builtin_cdr(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.size() != 1) throw LispError("cdr: expects 1 argument");
    if(!params[0]->isPair()) throw LispError("cdr: argument must be a pair. Got: " + params[0]->toString());
    return std::static_pointer_cast<PairValue>(params[0])->r;
}
static ValuePtr builtin_cons(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.size() != 2) throw LispError("cons: expects 2 arguments");
    return std::make_shared<PairValue>(params[0], params[1]);
}
static ValuePtr builtin_length(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if(params.size() != 1) throw LispError("length: expects 1 argument");
    if(!params[0]->isList() && !params[0]->isNil()){ // 确保是 proper list 或 nil
        throw LispError("length: argument must be a proper list or nil. Got: " + params[0]->toString());
    }
    if (params[0]->isNil()) return std::make_shared<NumericValue>(0.0);
    return std::make_shared<NumericValue>(static_cast<double>(params[0]->toVector().size()));
}
static ValuePtr builtin_list(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    std::vector<ValuePtr> elements = params; 
    return toList(elements);
}

static ValuePtr builtin_map(const std::vector<ValuePtr>& params, EvalEnv& env) {
    if(params.size() != 2){
        throw LispError("map: Exactly 2 arguments required.");
    }
    ValuePtr proc_object = params[0];
    ValuePtr list_object = params[1];
    if (!proc_object->isProcedure()) {
        throw LispError("map: first argument must be a procedure. Got: " + proc_object->toString());
    }
    if (!list_object->isList()) { 
        throw LispError("map: second argument must be a list. Got: " + list_object->toString());
    }
    if (list_object->isNil()) { 
        return LISP_NIL;
    }
    auto elements_to_map = list_object->toVector(); 
    std::vector<ValuePtr> result_elements;
    result_elements.reserve(elements_to_map.size());

    for(const auto& item : elements_to_map){
        result_elements.push_back(env.apply(proc_object, {item}));
    }
    return toList(result_elements);
}

static ValuePtr builtin_filter(const std::vector<ValuePtr>& params, EvalEnv& env) {
    if(params.size() != 2){
        throw LispError("filter: Exactly 2 arguments required.");
    }
    ValuePtr pred_object = params[0];
    ValuePtr list_object = params[1];
    if (!pred_object->isProcedure()) {
        throw LispError("filter: first argument must be a procedure. Got: " + pred_object->toString());
    }
    if (!list_object->isList()) {
        throw LispError("filter: second argument must be a list. Got: " + list_object->toString());
    }
    if (list_object->isNil()) {
        return LISP_NIL;
    }
    auto elements_to_filter = list_object->toVector();
    std::vector<ValuePtr> result_elements;
    for(const auto& item : elements_to_filter){
        ValuePtr predicate_result = env.apply(pred_object, {item});
        if (!predicate_result->isLispFalse()) {
            result_elements.push_back(item);
        }
    }
    return toList(result_elements);
}

static ValuePtr builtin_reduce(const std::vector<ValuePtr>& evaluated_args, EvalEnv& env) {
    if(evaluated_args.size() != 2){
        throw LispError("reduce: Exactly 2 values required.");
    }
    ValuePtr proc_object = evaluated_args[0];
    ValuePtr list_object = evaluated_args[1];
    if(!proc_object->isProcedure() || !list_object->isList() || list_object->isNil()){
        throw LispError("reduce: Invalid variables."); 
    }
    std::vector<ValuePtr> elements = list_object->toVector();
    if (elements.empty()) {
        throw LispError("reduce: Invalid variables."); 
    }
    ValuePtr accumulator = elements[0];
    if (elements.size() == 1) {
        return accumulator;
    }
    for (size_t i = 1; i < elements.size(); ++i) {
        accumulator = env.apply(proc_object, {accumulator, elements[i]});
    }
    return accumulator;
}

static ValuePtr builtin_add(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    double sum_result = 0.0;
    if (params.empty()) return std::make_shared<NumericValue>(0.0);
    for (const auto& arg_ptr : params) {
        if (!arg_ptr->isNumber()) throw LispError("+: expects numeric arguments. Got: " + arg_ptr->toString());
        sum_result += arg_ptr->asNumber(); // 假设 asNumber() 返回 double
    }
    return std::make_shared<NumericValue>(sum_result);
}
static ValuePtr builtin_subtract(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.empty()) throw LispError("-: expects at least 1 argument");
    if (!params[0]->isNumber()) throw LispError("-: expects numeric arguments. Got: " + params[0]->toString());
    double result = params[0]->asNumber();
    if (params.size() == 1) return std::make_shared<NumericValue>(-result);
    for (size_t i = 1; i < params.size(); ++i) {
        if (!params[i]->isNumber()) throw LispError("-: expects numeric arguments. Got: " + params[i]->toString());
        result -= params[i]->asNumber();
    }
    return std::make_shared<NumericValue>(result);
}
static ValuePtr builtin_multiply(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    double product_result = 1.0;
    if (params.empty()) return std::make_shared<NumericValue>(1.0);
    for (const auto& arg_ptr : params) {
        if (!arg_ptr->isNumber()) throw LispError("*: expects numeric arguments. Got: " + arg_ptr->toString());
        product_result *= arg_ptr->asNumber();
    }
    return std::make_shared<NumericValue>(product_result);
}
static ValuePtr builtin_divide(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.empty()) throw LispError("/: expects at least 1 argument");
    if (!params[0]->isNumber()) throw LispError("/: expects numeric arguments. Got: " + params[0]->toString());
    double result = params[0]->asNumber();
    if (params.size() == 1) {
        if (result == 0.0) throw LispError("/: division by zero");
        return std::make_shared<NumericValue>(1.0 / result);
    }
    for (size_t i = 1; i < params.size(); ++i) {
        if (!params[i]->isNumber()) throw LispError("/: expects numeric arguments. Got: " + params[i]->toString());
        double divisor = params[i]->asNumber();
        if (divisor == 0.0) throw LispError("/: division by zero");
        result /= divisor;
    }
    return std::make_shared<NumericValue>(result);
}
static ValuePtr builtin_abs(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=1) throw LispError("abs: expects 1 argument");
    if (!params[0]->isNumber()) throw LispError("abs: expects a numeric argument. Got: " + params[0]->toString());
    return std::make_shared<NumericValue>(std::abs(params[0]->asNumber()));
}

static ValuePtr builtin_expt(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=2) throw LispError("expt: expects 2 arguments (base exponent)");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("expt: expects numeric arguments.");
    return std::make_shared<NumericValue>(std::pow(params[0]->asNumber(), params[1]->asNumber()));
}
static ValuePtr builtin_quotient(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=2) throw LispError("quotient: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("quotient: expects numeric arguments.");
    double n1 = params[0]->asNumber();
    double n2 = params[1]->asNumber();
    if (n2 == 0.0) throw LispError("quotient: division by zero");
    return std::make_shared<NumericValue>(static_cast<double>(static_cast<long long>(n1 / n2)));
}
static ValuePtr builtin_modulo(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 2) throw LispError("modulo: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("modulo: expects numeric arguments.");
    double n1 = params[0]->asNumber();
    double n2 = params[1]->asNumber();
    if (n2 == 0.0) throw LispError("modulo: division by zero");
    double result = std::fmod(n1, n2);
    if (result * n2 < 0) { 
        result += n2;
    }
    return std::make_shared<NumericValue>(result);
}
static ValuePtr builtin_remainder(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 2) throw LispError("remainder: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("remainder: expects numeric arguments.");
    double n1 = params[0]->asNumber();
    double n2 = params[1]->asNumber();
    if (n2 == 0.0) throw LispError("remainder: division by zero");
    return std::make_shared<NumericValue>(std::fmod(n1, n2));
}
static ValuePtr builtin_eq(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 2) {
        throw LispError("eq?: expects 2 arguments");
    }
    ValuePtr p1 = params[0];
    ValuePtr p2 = params[1];
    bool result = (p1.get() == p2.get());
    if (!result && p1->isNumber() && p2->isNumber()) {
        if (std::abs(p1->asNumber() - p2->asNumber()) < 1e-9) { 
            result = true;
        }
    }
    return result ? LISP_TRUE : LISP_FALSE;
}
static bool are_values_equal_recursive(ValuePtr p1, ValuePtr p2) {
    if (p1.get() == p2.get()) {
        return true;
    }
    if (p1->isPair() && p2->isPair()) {
        auto pair1 = std::static_pointer_cast<PairValue>(p1);
        auto pair2 = std::static_pointer_cast<PairValue>(p2);
        return are_values_equal_recursive(pair1->l, pair2->l) &&
               are_values_equal_recursive(pair1->r, pair2->r);
    }
    if (p1->isNil() && p2->isNil()) {
        return true;
    }
    if (p1->isNumber() && p2->isNumber()) {
        return std::abs(p1->asNumber() - p2->asNumber()) < 1e-9;
    }
    if (p1->isString() && p2->isString()) {
        return p1->asString() == p2->asString();
    }
    if (p1->isSymbol() && p2->isSymbol()) {
        return p1.get() == p2.get();
    }
    if (p1->isBoolean() && p2->isBoolean()) {
        return p1.get() == p2.get();
    }
    return false;
}
static ValuePtr builtin_equal_(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size() != 2) {
        throw LispError("equal?: expects 2 arguments");
    }
    bool result = are_values_equal_recursive(params[0], params[1]);
    return result ? LISP_TRUE : LISP_FALSE;
}
static ValuePtr builtin_not(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) { 
    if (params.size()!=1) throw LispError("not: expects 1 argument");
    return (params[0]->isLispFalse())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_greater(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=2) throw LispError(">: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError(">: expects numeric arguments.");
    return (params[0]->asNumber() > params[1]->asNumber())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_lesser(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=2) throw LispError("<: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("<: expects numeric arguments.");
    return (params[0]->asNumber() < params[1]->asNumber())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_equal(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) { // Numeric equality
    if (params.size()!=2) throw LispError("=: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("=: expects numeric arguments.");
    return (params[0]->asNumber() == params[1]->asNumber())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_greater_equal(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=2) throw LispError(">=: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError(">=: expects numeric arguments.");
    return (params[0]->asNumber() >= params[1]->asNumber())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_lesser_equal(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=2) throw LispError("<=: expects 2 arguments");
    if (!params[0]->isNumber() || !params[1]->isNumber()) throw LispError("<=: expects numeric arguments.");
    return (params[0]->asNumber() <= params[1]->asNumber())?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_is_even(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=1) throw LispError("even?: expects 1 argument");
    if (!params[0]->isNumber()) throw LispError("even?: expects a numeric argument.");
    double val = params[0]->asNumber();
    if (std::trunc(val) != val) throw LispError("even?: expects an integer argument.");
    return (static_cast<long long>(val) % 2 == 0)?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_is_odd(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=1) throw LispError("odd?: expects 1 argument");
    if (!params[0]->isNumber()) throw LispError("odd?: expects a numeric argument.");
    double val = params[0]->asNumber();
    if (std::trunc(val) != val) throw LispError("odd?: expects an integer argument.");
    return (static_cast<long long>(val) % 2 != 0)?LISP_TRUE:LISP_FALSE;
}
static ValuePtr builtin_is_zero(const std::vector<ValuePtr>& params, EvalEnv& env /*env unused*/) {
    if (params.size()!=1) throw LispError("zero?: expects 1 argument");
    if (!params[0]->isNumber()) throw LispError("zero?: expects a numeric argument.");
    return (params[0]->asNumber() == 0.0)?LISP_TRUE:LISP_FALSE;
}
const BuiltinProceduresMap& get_builtin_procedures() {
    static BuiltinProceduresMap procedures_map_instance; 
    static bool initialized = false;
    if (!initialized) {
        procedures_map_instance["apply"] = std::make_shared<BuiltinProcValue>(&builtin_apply);
        procedures_map_instance["display"] = std::make_shared<BuiltinProcValue>(&builtin_display);
        procedures_map_instance["displayln"] = std::make_shared<BuiltinProcValue>(&builtin_displayln);
        procedures_map_instance["error"] = std::make_shared<BuiltinProcValue>(&builtin_error);
        procedures_map_instance["eval"] = std::make_shared<BuiltinProcValue>(&builtin_eval);
        procedures_map_instance["exit"] = std::make_shared<BuiltinProcValue>(&builtin_exit);
        procedures_map_instance["newline"] = std::make_shared<BuiltinProcValue>(&builtin_newline);
        procedures_map_instance["print"] = std::make_shared<BuiltinProcValue>(&builtin_print);
        procedures_map_instance["atom?"] = std::make_shared<BuiltinProcValue>(&builtin_atom);
        procedures_map_instance["boolean?"] = std::make_shared<BuiltinProcValue>(&builtin_boolean);
        procedures_map_instance["integer?"] = std::make_shared<BuiltinProcValue>(&builtin_integer);
        procedures_map_instance["list?"] = std::make_shared<BuiltinProcValue>(&builtin_list_);
        procedures_map_instance["number?"] = std::make_shared<BuiltinProcValue>(&builtin_number);
        procedures_map_instance["null?"] = std::make_shared<BuiltinProcValue>(&builtin_null);
        procedures_map_instance["pair?"] = std::make_shared<BuiltinProcValue>(&builtin_pair);
        procedures_map_instance["procedure?"] = std::make_shared<BuiltinProcValue>(&builtin_procedure);
        procedures_map_instance["string?"] = std::make_shared<BuiltinProcValue>(&builtin_string);
        procedures_map_instance["symbol?"] = std::make_shared<BuiltinProcValue>(&builtin_symbol);
        procedures_map_instance["append"] = std::make_shared<BuiltinProcValue>(&builtin_append);
        procedures_map_instance["car"] = std::make_shared<BuiltinProcValue>(&builtin_car);
        procedures_map_instance["cdr"] = std::make_shared<BuiltinProcValue>(&builtin_cdr);
        procedures_map_instance["cons"] = std::make_shared<BuiltinProcValue>(&builtin_cons);
        procedures_map_instance["length"] = std::make_shared<BuiltinProcValue>(&builtin_length);
        procedures_map_instance["list"] = std::make_shared<BuiltinProcValue>(&builtin_list);
        procedures_map_instance["map"] = std::make_shared<BuiltinProcValue>(&builtin_map);
        procedures_map_instance["filter"] = std::make_shared<BuiltinProcValue>(&builtin_filter);
        procedures_map_instance["reduce"] = std::make_shared<BuiltinProcValue>(&builtin_reduce);
        procedures_map_instance["+"] = std::make_shared<BuiltinProcValue>(&builtin_add);
        procedures_map_instance["-"] = std::make_shared<BuiltinProcValue>(&builtin_subtract);
        procedures_map_instance["*"] = std::make_shared<BuiltinProcValue>(&builtin_multiply);
        procedures_map_instance["/"] = std::make_shared<BuiltinProcValue>(&builtin_divide);
        procedures_map_instance["abs"] = std::make_shared<BuiltinProcValue>(&builtin_abs);
        procedures_map_instance["expt"] = std::make_shared<BuiltinProcValue>(&builtin_expt);
        procedures_map_instance["quotient"] = std::make_shared<BuiltinProcValue>(&builtin_quotient);
        procedures_map_instance["modulo"] = std::make_shared<BuiltinProcValue>(&builtin_modulo);
        procedures_map_instance["remainder"] = std::make_shared<BuiltinProcValue>(&builtin_remainder);
        procedures_map_instance["eq?"] = std::make_shared<BuiltinProcValue>(&builtin_eq);
        procedures_map_instance["equal?"] = std::make_shared<BuiltinProcValue>(&builtin_equal_);
        procedures_map_instance["not"] = std::make_shared<BuiltinProcValue>(&builtin_not);
        procedures_map_instance[">"] = std::make_shared<BuiltinProcValue>(&builtin_greater);
        procedures_map_instance["<"] = std::make_shared<BuiltinProcValue>(&builtin_lesser);
        procedures_map_instance["="] = std::make_shared<BuiltinProcValue>(&builtin_equal);
        procedures_map_instance[">="] = std::make_shared<BuiltinProcValue>(&builtin_greater_equal);
        procedures_map_instance["<="] = std::make_shared<BuiltinProcValue>(&builtin_lesser_equal);
        procedures_map_instance["even?"] = std::make_shared<BuiltinProcValue>(&builtin_is_even);
        procedures_map_instance["odd?"] = std::make_shared<BuiltinProcValue>(&builtin_is_odd);
        procedures_map_instance["zero?"] = std::make_shared<BuiltinProcValue>(&builtin_is_zero);
        initialized = true;
    }
    return procedures_map_instance;
}