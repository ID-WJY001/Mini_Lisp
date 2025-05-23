// file: forms.h
#ifndef FORMS_H
#define FORMS_H

#include "value.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

class EvalEnv;

using SpecialFormType = ValuePtr(const std::vector<ValuePtr>& args, EvalEnv& env);

extern const std::unordered_map<std::string, SpecialFormType*> SPECIAL_FORMS;
ValuePtr beginForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr condForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr defineForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr letForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr quoteForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr quasiquoteForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr unquoteForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr ifForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr andForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr orForm(const std::vector<ValuePtr>& args, EvalEnv& env);
ValuePtr lambdaForm(const std::vector<ValuePtr>& args, EvalEnv& env);

#endif // FORMS_H