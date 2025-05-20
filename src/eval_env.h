// file: eval_env.h
#ifndef EVAL_ENV_H  // 如果 EVAL_ENV_H 宏未定义
#define EVAL_ENV_H  // 则定义 EVAL_ENV_H 宏

#include <unordered_map>
#include "./value.h"
#include "./builtins.h"
// #include "./forms.h" // 暂时注释掉，或者确保 forms.h 也正确处理了前向声明

// 如果 forms.h 只需要 EvalEnv 的前向声明，而 EvalEnv 的定义不需要 SPECIAL_FORMS 的具体内容
// 可以在这里前向声明 SPECIAL_FORMS 相关类型，而不是直接包含 forms.h
// 或者，如果 EvalEnv 的方法体会用到 SPECIAL_FORMS，则包含 forms.h 是必要的，但 forms.h 必须正确处理依赖。
// 考虑到你的 EvalEnv::eval 中直接使用了 SPECIAL_FORMS，包含 forms.h 是合理的。
// 重点是 forms.h 也必须有包含保护符，并且对 EvalEnv 使用前向声明。
#include "./forms.h"


class EvalEnv {
public:
    EvalEnv();
    EvalEnv(const EvalEnv& v)=default;
    std::unordered_map<std::string,ValuePtr> symbol_map{};
    ValuePtr eval(const ValuePtr &expr);
    std::vector<ValuePtr> evalList(ValuePtr expr); // 建议改为 const ValuePtr& 或 ValuePtr&
    ValuePtr apply(ValuePtr proc, std::vector<ValuePtr> args);
};

#endif // EVAL_ENV_H