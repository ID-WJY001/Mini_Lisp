#include <memory>
#include "parser.h"
#include "token.h"
#include "./error.h"
ValuePtr Parser::parse() {
    if (tokens.empty()) {
        throw SyntaxError("Unexpected end of input: no token to parse.");
    }
    TokenPtr token = std::move(tokens.front());
    tokens.pop_front();
    switch (token->getType()) {
        case TokenType::NUMERIC_LITERAL: {
            auto value = static_cast<NumericLiteralToken&>(*token).getValue();
            return std::make_shared<NumericValue>(value);
        }
        case TokenType::BOOLEAN_LITERAL: {
            auto value = static_cast<BooleanLiteralToken&>(*token).getValue();
            return std::make_shared<BooleanValue>(value);
        }
        case TokenType::STRING_LITERAL: {
            auto value = static_cast<StringLiteralToken&>(*token).getValue();
            return std::make_shared<StringValue>(value);
        }
        case TokenType::IDENTIFIER: {
            auto value = static_cast<IdentifierToken&>(*token).getName();
            return std::make_shared<SymbolValue>(value);
        }
        case TokenType::LEFT_PAREN: {
            return this->parseTails();
        }
        case TokenType::QUOTE: {
            if (tokens.empty()) {
                throw SyntaxError("Unexpected end of input after ' (quote). Expected an expression.");
            }
            ValuePtr expr = this->parse();
            return std::make_shared<PairValue>(
                std::make_shared<SymbolValue>("quote"),
                std::make_shared<PairValue>(expr, std::make_shared<NilValue>())
            );
        }
        case TokenType::QUASIQUOTE: {
            if (tokens.empty()) {
                throw SyntaxError("Unexpected end of input after ` (quasiquote). Expected an expression.");
            }
            ValuePtr expr = this->parse();
            return std::make_shared<PairValue>(
                std::make_shared<SymbolValue>("quasiquote"),
                std::make_shared<PairValue>(expr, std::make_shared<NilValue>())
            );
        }
        case TokenType::UNQUOTE: {
            if (tokens.empty()) {
                throw SyntaxError("Unexpected end of input after , (unquote). Expected an expression.");
            }
            ValuePtr expr = this->parse();
            return std::make_shared<PairValue>(
                std::make_shared<SymbolValue>("unquote"),
                std::make_shared<PairValue>(expr, std::make_shared<NilValue>())
            );
        }
        case TokenType::RIGHT_PAREN:
            throw SyntaxError("Unexpected ')' token encountered. It should only appear within a list structure handled by parseTails.");
        case TokenType::DOT:
            throw SyntaxError("Unexpected '.' token encountered. It should only appear within a list structure handled by parseTails.");
        default: {
            throw SyntaxError("Unimplemented or unexpected token type encountered by parse().");
        }
    }
}
ValuePtr Parser::parseTails() {
    if (tokens.empty()) {
        throw SyntaxError("Unexpected end of input: expected ')' or an element for list/pair tail, but stream is empty.");
    }
    if (tokens.front()->getType() == TokenType::RIGHT_PAREN) {
        tokens.pop_front();
        return std::make_shared<NilValue>();
    }
    ValuePtr car = this->parse();
    if (tokens.empty()) {
        throw SyntaxError("Unexpected end of input after parsing CAR of a list/pair: expected '.' or another element or ')'.");
    }
    if (tokens.front()->getType() == TokenType::DOT) {
        tokens.pop_front();
        if (tokens.empty()) {
            throw SyntaxError("Unexpected end of input after '.' in a dotted pair. Expected CDR expression.");
        }
        ValuePtr cdr = this->parse();
        if (tokens.empty()) {
            throw SyntaxError("Unexpected end of input after parsing CDR of a dotted pair. Expected ')'.");
        }
        if (tokens.front()->getType() != TokenType::RIGHT_PAREN) {
            throw SyntaxError("Syntax error: expected ')' after CDR in dotted pair, but got other token.");
        }
        tokens.pop_front();
        return std::make_shared<PairValue>(car, cdr);
    } 
    else {
        ValuePtr cdr_list_part = this->parseTails();
        return std::make_shared<PairValue>(car, cdr_list_part);
    }
}