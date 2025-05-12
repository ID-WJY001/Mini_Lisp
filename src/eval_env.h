#include <unordered_map>
#include "./value.h"

class EvalEnv {
public:
    std::unordered_map<std::string,ValuePtr> symbol_map{};
    ValuePtr eval(ValuePtr expr);
};