#pragma once
#include "value.h"
#include "error.h"
#include "token.h"

#include <map>

using BuiltinProceduresMap = std::map<std::string, std::shared_ptr<BuiltinProcValue>>;

const BuiltinProceduresMap& get_builtin_procedures();
