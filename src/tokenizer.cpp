#include "./tokenizer.h"

#include <cctype>
#include <set>
#include <stdexcept>

#include "./error.h"

const std::set<char> TOKEN_END{'(', ')', '\'', '`', ',', '"'};
std::deque<TokenPtr> Tokenizer::tokenize() {
    std::deque<TokenPtr> tokens;
    int pos = 0;
    while (true) {
        auto token = nextToken(pos);
        if (!token) {
            break;
        }
        tokens.push_back(std::move(token));
    }
    return tokens;
}

std::deque<TokenPtr> Tokenizer::tokenize(const std::string& input) {
    return Tokenizer(input).tokenize();
}
TokenPtr Tokenizer::nextToken(int& pos) {
    while (pos < input.size()) {
        bool comment_handled = false; // 【新增】布尔标志位

        auto c = input[pos];

        if (std::isspace(c)) {
            pos++;
            continue;
        }

        if (c == ';') {
            while (pos < input.size() && input[pos] != '\n') {
                pos++;
            }
            continue;
        }

        if (c == '#') {
            if (pos + 1 < input.size() && input[pos + 1] == '|') {
                pos += 2; 
                while (pos + 1 < input.size()) {
                    if (input[pos] == '|' && input[pos + 1] == '#') {
                        pos += 2; 
                        comment_handled = true; // 【修改】设置标志位
                        break; // 跳出内部的 while 循环
                    }
                    pos++;
                }

                if (!comment_handled) { // 如果循环结束了还没找到闭合
                    throw SyntaxError("Unterminated block comment starting with #|");
                }
            } else if (auto result = BooleanLiteralToken::fromChar(input[pos + 1])) {
                pos += 2;
                return result;
            } else {
                throw SyntaxError("Unexpected character after #. Expected 't', 'f', or '|'.");
            }
        }
        
        // 【新增】检查标志位。如果是 true，说明刚处理完注释，需要开始新一轮外层循环
        if (comment_handled) {
            continue;
        }

        // --- 从这里开始，是原来的 token 解析逻辑，完全不变 ---
        
        if (auto token = Token::fromChar(c)) {
            pos++;
            return token;
        }

        if (c == '"') {
            std::string string;
            pos++;
            while (pos < input.size()) {
                if (input[pos] == '"') {
                    pos++;
                    return std::make_unique<StringLiteralToken>(string);
                } else if (input[pos] == '\\') {
                    if (pos + 1 >= input.size()) {
                        throw SyntaxError("Unexpected end of string literal");
                    }
                    auto next = input[pos + 1];
                    if (next == 'n') {
                        string += '\n';
                    } else {
                        string += next;
                    }
                    pos += 2;
                } else {
                    string += input[pos];
                    pos++;
                }
            }
            throw SyntaxError("Unexpected end of string literal");
        }

        int start = pos;
        do {
            pos++;
        } while (pos < input.size() && !std::isspace(input[pos]) &&
                 TOKEN_END.find(input[pos]) == TOKEN_END.end());
        
        auto text = input.substr(start, pos - start);

        if (text == ".") {
            return Token::dot();
        }

        if (std::isdigit(text[0]) || (text.length() > 1 && (text[0] == '+' || text[0] == '-')) || text[0] == '.') {
            try {
                size_t processed_chars;
                double num = std::stod(text, &processed_chars);
                if (processed_chars == text.length()) {
                    return std::make_unique<NumericLiteralToken>(num);
                }
            } catch (const std::invalid_argument&) {
                // Not a number, treat as an identifier
            }
        }
        
        return std::make_unique<IdentifierToken>(text);
    } 

    return nullptr;
}