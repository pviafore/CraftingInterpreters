#ifndef LOXCPP_COMPILER_H_
#define LOXCPP_COMPILER_H_

#include <functional>
#include <limits>

#include "chunk.h"
#include "optional.h"
#include "parser.h"
#include "string.h"
#include "vector.h"
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
        void block();
        void beginScope();
        void endScope();
        void printStatement();
        void expressionStatement();
        void varDeclaration(bool constant = false);
        void constDeclaration();
        void variable(bool);
        size_t previousLine() const;
        void parsePrecedence(Precedence precedence);
        const ParseRule& getRule(TokenType type) const;
        size_t parseVariable(StringView errorMessage, bool constant);
        size_t addIdentifierConstant(StringView name, bool constant);
        void defineVariable(size_t global);
        void declareVariable(bool constant);
        void addLocal(StringView name, bool constant);
        Optional<size_t> resolveLocal(StringView name);
        void markInitialized();
        Scanner scanner;
        Parser parser;
        Chunk chunk;
        Table<InternedString, size_t> constants;
        HashSet<InternedString> immutables;

        struct Local {
            StringView name;
            Optional<size_t> depth;  // will not have a value if its uninitialized
            bool constant;
        };
        StaticVector<Local, std::numeric_limits<uint32_t>::max()> locals;
        size_t localCount = 0;
        size_t depth = 0;
    };
}
#endif