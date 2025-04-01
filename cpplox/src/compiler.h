#ifndef LOXCPP_COMPILER_H_
#define LOXCPP_COMPILER_H_

#include <functional>

#include "chunk.h"
#include "optional.h"
#include "parser.h"
#include "string.h"
namespace lox {

    enum class Precedence {
        None,
        Assignment,
        Ternary,
        Or,
        And,
        Equality,
        Comparison,
        Term,
        Factor,
        Unary,
        Call,
        Primary
    };
    class Compiler {
    public:
        Compiler(const String& s);
        Optional<Chunk> compile();
        bool debugMode = false;

    private:
        using ParseFn = std::function<void(Compiler*)>;
        struct ParseRule {
            ParseFn prefix{};
            ParseFn infix{};
            Precedence precedence = Precedence::None;
        };

        Chunk& getCurrentChunk();
        void emit(std::byte byte);
        void emit(OpCode b1, std::byte b2);
        void emit(OpCode b1);
        void emitConstant(Value value);
        void number();
        void string();
        void expression();
        void grouping();
        void unary();
        void binary();
        void ternary();
        void literal();
        size_t previousLine() const;
        void parsePrecedence(Precedence precedence);
        const ParseRule& getRule(TokenType type) const;
        Scanner scanner;
        Parser parser;
        Chunk chunk;
    };
}
#endif