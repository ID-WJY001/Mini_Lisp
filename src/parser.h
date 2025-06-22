#ifndef PARSER_H
#define PARSER_H

#include <deque>
#include "value.h"
#include "token.h"

class Parser {
private:
    std::deque<TokenPtr> tokens;
public:
    Parser(std::deque<TokenPtr>tokens):tokens(std::move(tokens)){}
    ValuePtr parse();
    ValuePtr parseTails();
    bool isAtEnd() const;
};

#endif