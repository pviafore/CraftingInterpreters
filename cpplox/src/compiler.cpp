#include "compiler.h"

#include <print>

#include "scanner.h"
namespace lox {
    void Compiler::compile(const String& s) {
        Scanner scanner(s);
        size_t line = 0;
        for (auto token : scanner) {
            if (token.line != line) {
                std::print("{:4}", token.line);
                line = token.line;
            } else {
                std::print("   | ");
            }
            std::println("{}", token);
            if (token.type == TokenType::Eof) {
                break;
            }
        }
    }
}