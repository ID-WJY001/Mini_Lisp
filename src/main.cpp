#include <iostream>
#include <string>
#include "rjsj_test.hpp"
#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"
struct TestCtx {
    std::shared_ptr<EvalEnv> env = std::make_shared<EvalEnv>(); 
    TestCtx() = default; 
    std::string eval(std::string input) {
        auto tokens = Tokenizer::tokenize(input);
        Parser parser(std::move(tokens));
        auto value = parser.parse();
        auto result = env->eval(std::move(value));
        return result->toString();
    }
};
int main() {
    RJSJ_TEST(TestCtx, Lv2, Lv3, Lv4, Lv5, Lv5Extra, Lv6);
    auto env = std::make_shared<EvalEnv>();
    while (true) {
        try {
            std::cout << ">>> " ;
            std::string line;
            std::getline(std::cin, line);
            if (std::cin.eof()) {
                std::exit(0);
            }
            auto tokens = Tokenizer::tokenize(line);
            Parser parser(std::move(tokens));
            auto value = parser.parse();
            auto result = env->eval(value);
            std::cout << result->toString() << std::endl;
            /*for (auto& token : tokens) {
                std::cout << *token << std::endl;
            }*/
        } catch (std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}