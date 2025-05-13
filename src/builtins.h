#pragma once
#include "value.h"
#include "error.h"
#include "token.h"

#include <unordered_map>

using BuiltinProceduresMap = std::unordered_map<std::string, ValuePtr>;

const BuiltinProceduresMap& get_builtin_procedures();

ValuePtr add(const std::vector<ValuePtr>& params);
