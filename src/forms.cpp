#include "forms.h"
#include "error.h"
#include "eval_env.h"
#include "value.h"
#include <iostream>

ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 2) {
        throw LispError("Invalid Definition: define needs only 2 values.");
    }

    auto var_name_opt = args[0]->asSymbol();
    if (!var_name_opt) {
        throw LispError("Invalid Definition: define value error.");
    }
    const std::string& variable_name = *var_name_opt;
    ValuePtr value_to_be_evaluated = args[1];
    ValuePtr evaluated_value = env.eval(value_to_be_evaluated);
    env.symbol_map[variable_name] = evaluated_value; 
    return std::make_shared<NilValue>();
}

ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 1) {
        throw LispError("Invalid Quote: quote needs only one value.");
    }
    return args[0];
}

/*ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    if (args.size() != 3) {
        throw LispError("Invalid Definition: if needs only 3 values.");
    }
    auto first = std::make_shared<BooleanValue>(args[0]);
    if (first = std::make_shared<BooleanValue>(1)) {
        return std::make_shared<Value>(args[1]);
    }
    else{
        return std::make_shared<Value>(args[2]);
    }
}

ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for(auto i : args){
        if(std::make_shared<NilValue>(i) == std::make_shared<NilValue>(0)){
            return std::make_shared<NilValue>(0);
        }
    }
    return std::make_shared<Value>(args.end());
}

ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env) {
    for(auto i : args){
        if(std::make_shared<NilValue>(i) == std::make_shared<NilValue>(1)){
            return std::make_shared<Value>(i);
        }
    }
    return std::make_shared<NilValue>(0);
}*/

const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS{
    {"define", defineForm}, 
    {"quote",  quoteForm}/*,
    {"if", ifForm},
    {"and", andForm},
    {"or", orForm}*/
};