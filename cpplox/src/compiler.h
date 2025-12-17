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

    struct ClassCompiler {
        ClassCompiler* enclosing;
        bool hasSuperclass = false;
    };
    class Compiler {
    public:
        enum FunctionType {
            FUNCTION,
            METHOD,
            INITIALIZER,
            SCRIPT
        };

        Compiler(const String& s);
        SharedPtr<Function> compile();
        bool debugMode = true;

    private:
        Compiler(Compiler* compiler, FunctionType type);
        using ParseFn = std::function<void(Compiler*, bool)>;
        struct ParseRule {
            ParseFn prefix{};
            ParseFn infix{};
            Precedence precedence = Precedence::None;
        };

        SharedPtr<Chunk> getCurrentChunk();
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
        void dot(bool);
        void unary(bool);
        void binary(bool);
        void ternary(bool);
        void literal(bool);
        void andOp(bool);
        void orOp(bool);
        void call(bool);
        void this_(bool);
        void inner(bool);
        uint8_t argumentList();

        void declaration();
        void statement();
        void block();

        void beginScope();
        void endScope();
        void printStatement();
        void ifStatement();
        void whileStatement();
        void forStatement();
        void switchStatement();
        void breakStatement();
        void continueStatement();
        void onceStatement();
        void method();
        void returnStatement();
        void emitLoop(size_t pos);
        void emitReturn();
        size_t emitJump(OpCode opCode);
        void patchJump(size_t pos);
        void expressionStatement();
        StringView varDeclaration(bool constant = false);
        void constDeclaration();
        void funDeclaration();
        void variable(bool);
        void classDeclaration();
        void func(FunctionType ft);
        size_t previousLine() const;
        void parsePrecedence(Precedence precedence);
        const ParseRule& getRule(TokenType type) const;
        size_t parseVariable(StringView errorMessage, bool constant);
        size_t addIdentifierConstant(StringView name, bool constant);
        void defineVariable(size_t global);
        void declareVariable(bool constant);
        size_t addLocal(StringView name, bool constant);
        Optional<size_t> resolveLocal(StringView name);
        Optional<size_t> resolveUpvalue(StringView name);
        size_t addUpvalue(size_t index, bool isLocal);
        void setupOnceTracking();
        void markInitialized();
        void beginCompile();
        SharedPtr<Function> endCompile();
        String manglePrivate(StringView name);
        Scanner scanner;
        SharedPtr<Parser> parser;
        Table<InternedString, size_t> constants;
        HashSet<String> immutables;

        struct Local {
            InternedString name;
            Optional<size_t> depth;  // will not have a value if its uninitialized
            bool constant;
            bool isCaptured = false;
        };
        StaticVector<Local, 1024> locals;
        size_t localCount = 0;
        size_t depth = 0;

        size_t onceTracker = 0;
        size_t numberOfOnces = 0;

        SharedPtr<Function> function;
        FunctionType functionType = FunctionType::SCRIPT;
        Compiler* enclosing = nullptr;

        struct Loop {
            size_t depth = 0;
            size_t startLocation = 0;
            Vector<size_t> breakLocations = {};
        };
        Vector<Loop> nestedLoops;

        ClassCompiler* classCompiler = nullptr;
        StringView currentClass = "";
        StringView methodName = "";
    };
}
#endif