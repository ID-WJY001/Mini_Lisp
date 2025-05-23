#ifndef EVAL_ENV_H  // 如果 EVAL_ENV_H 宏未定义
#define EVAL_ENV_H  // 则定义 EVAL_ENV_H 宏

#include <map>
#include "./value.h"
#include "./builtins.h"
#include "./forms.h"

extern const ValuePtr LISP_NIL;
extern const ValuePtr LISP_TRUE;
extern const ValuePtr LISP_FALSE;
extern std::unordered_map<std::string, ValuePtr> global_symbol_table;
ValuePtr create_or_get_symbol(const std::string& name);

class EvalEnv : public std::enable_shared_from_this<EvalEnv>{
public:
    std::shared_ptr<EvalEnv> parent = nullptr;
    EvalEnv();
    EvalEnv(std::shared_ptr<EvalEnv> parent_env) : parent(parent_env) {}
    EvalEnv(const EvalEnv& v)=default;
    std::map<std::string,ValuePtr> symbol_map{};
    ValuePtr eval(const ValuePtr &expr);
    std::vector<ValuePtr> evalList(ValuePtr expr);
    ValuePtr apply(ValuePtr proc, std::vector<ValuePtr> args);
    ValuePtr lookupBinding(const std::string& name);
    void defineBinding(const std::string& name, ValuePtr value);
    std::shared_ptr<EvalEnv> get_shared_this() {
        return shared_from_this();
    }
};

#endif // EVAL_ENV_H