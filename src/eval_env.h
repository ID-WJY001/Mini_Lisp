#ifndef EVAL_ENV_H  // 如果 EVAL_ENV_H 宏未定义
#define EVAL_ENV_H  // 则定义 EVAL_ENV_H 宏

#include <unordered_map>
#include "./value.h"
#include "./builtins.h"
#include "./forms.h"

class EvalEnv : public std::enable_shared_from_this<EvalEnv>{
public:
    std::shared_ptr<EvalEnv> parent = nullptr;
    EvalEnv();
    EvalEnv(std::shared_ptr<EvalEnv> parent_env) : parent(parent_env) {}
    EvalEnv(const EvalEnv& v)=default;
    std::unordered_map<std::string,ValuePtr> symbol_map{};
    ValuePtr eval(const ValuePtr &expr);
    std::vector<ValuePtr> evalList(ValuePtr expr); // 建议改为 const ValuePtr& 或 ValuePtr&
    ValuePtr apply(ValuePtr proc, std::vector<ValuePtr> args);
    ValuePtr lookupBinding(const std::string& name);
    void defineBinding(const std::string& name, ValuePtr value);
    std::shared_ptr<EvalEnv> get_shared_this() {
        return shared_from_this();
    }
    std::shared_ptr<EvalEnv> createChild(const std::vector<std::string>& params, const std::vector<ValuePtr>& args);
};

#endif // EVAL_ENV_H