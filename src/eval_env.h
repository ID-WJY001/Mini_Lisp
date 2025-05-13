#include <unordered_map>
#include "./value.h"
#include "./builtins.h"

class EvalEnv {
public:
    EvalEnv();
    std::unordered_map<std::string,ValuePtr> symbol_map{};
    ValuePtr eval(ValuePtr expr);
    std::vector<ValuePtr> evalList(ValuePtr expr);
    ValuePtr apply(ValuePtr proc, std::vector<ValuePtr> args);
};