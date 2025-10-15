#include <iostream>
#include <string>
#include <fstream> 
#include <sstream>  
#include "rjsj_test.hpp"
#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"
#include "./error.h"

std::string readFileToString(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        std::cerr << "Error: Could not open file '" << filePath << "'" << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

struct TestCtx {
    std::shared_ptr<EvalEnv> env = std::make_shared<EvalEnv>(); 
    TestCtx() = default; 
    std::string eval(std::string input) {
        auto tokens = Tokenizer::tokenize(input);
        Parser parser(std::move(tokens));
        std::string last_result_str;
        while (!parser.isAtEnd()) {
            auto value = parser.parse();
            if (!value) break;
            auto result = env->eval(std::move(value));
            if (result) {
                 last_result_str = result->toString();
            }
        }
        return last_result_str;
    }
};

int main(int argc, char* argv[]) {
    //RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv5Extra, Lv6, Lv7, Lv7Lib, Sicp);

    auto env = std::make_shared<EvalEnv>();

    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [optional_filepath]" << std::endl;
        return 1; 
    }

    
    if (argc == 2) {
        std::string filePath = argv[1];
        std::string fileContent = readFileToString(filePath);

       
        if (fileContent.empty() && !std::ifstream(filePath).good()) {
            return 1; // 文件读取失败，直接退出
        }
        
        try {
            auto tokens = Tokenizer::tokenize(fileContent);
            Parser parser(std::move(tokens));

            // 循环执行文件中的所有表达式
            while (!parser.isAtEnd()) {
                auto value = parser.parse();
                if (!value) {
                    break;
                }
                // 只求值，不打印结果，除非遇到 display 等函数
                env->eval(value);
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1; // 执行出错，以非零状态码退出
        }
        
        return 0; // 文件成功执行完毕，正常退出
    }

    while (true) {
        std::string full_expression_str;
        int paren_balance = 0;
        bool first_line = true;

        do {
            if (first_line) {
                std::cout << ">>> ";
                first_line = false;
            } else {
                std::cout << "... ";
            }
            std::string current_line;
            if (!std::getline(std::cin, current_line)) {
                std::cout << std::endl;
                std::exit(0);
            }
            for (char c : current_line) {
                if (c == '(') paren_balance++;
                else if (c == ')') paren_balance--;
            }
            full_expression_str += current_line + "\n";
        } while (paren_balance > 0);
        
        try {
            if (paren_balance < 0) {
                throw LispError("Unmatched closing parenthesis ')'");
            }
            if (full_expression_str.find_first_not_of(" \t\n\r") == std::string::npos) {
                continue;
            }
            auto tokens = Tokenizer::tokenize(full_expression_str);
            Parser parser(std::move(tokens));
            ValuePtr last_result = nullptr; 
            while (!parser.isAtEnd()) {
                auto value = parser.parse();
                if (value == nullptr) {
                    break;
                }
                last_result = env->eval(value);
            }
            if (last_result) {
                std::cout << last_result->toString() << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}