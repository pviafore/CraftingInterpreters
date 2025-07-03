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
        using ParseFn = std::function<void(Compiler*, bool)>;
        struct ParseRule {
            ParseFn prefix{};
            ParseFn infix{};
            Precedence precedence = Precedence::None;
        };

        Chunk& getCurrentChunk();
        void emit(std::byte byte);
        void emit(OpCode b1, std::byte b2);
        void emit(OpCode b1);
        void emit(size_t val);
        void emitConstant(Value value);
        void emitNamedVariable(StringView name, bool canAssign);
        void number(bool);
        void string(bool);
        void expression();
        void grouping(bool);
        void unary(bool);
        void binary(bool);
        void ternary(bool);
        void literal(bool);

        void declaration();
        void statement();
        void printStatement();
        void expressionStatement();
        void varDeclaration();
        void variable(bool);
        size_t previousLine() const;
        void parsePrecedence(Precedence precedence);
        const ParseRule& getRule(TokenType type) const;
        size_t parseVariable(StringView errorMessage);
        size_t addIdentifierConstant(StringView name);
        void defineVariable(size_t global);
        Scanner scanner;
        Parser parser;
        Chunk chunk;
    };
}
#endif