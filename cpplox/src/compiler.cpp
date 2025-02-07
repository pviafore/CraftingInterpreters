#include "compiler.h"

#include <print>

#include "debug.h"

namespace lox {
    Compiler::Compiler(const String& s) : scanner(s), parser(scanner.begin()) {}
    Optional<Chunk> Compiler::compile() {
        expression();
        parser.consume(TokenType::Eof, "Expect end of expression");
        auto outChunk = parser.hasError() ? Optional<Chunk>{} : Optional{chunk};
        if (outChunk) {
            outChunk.value().write(OpCode::Return, parser.getPreviousToken().line);
        }
        if (debugMode && !parser.hasError()) {
            std::println("{}", chunk);
        }
        return outChunk;
    }

    void Compiler::emit(std::byte byte) {
        getCurrentChunk().write(byte, parser.getPreviousToken().line);
    }

    void Compiler::emit(OpCode b1, std::byte b2) {
        emit(b1);
        emit(b2);
    }

    void Compiler::emit(OpCode b1) {
        emit(std::byte{std::to_underlying(b1)});
    }

    Chunk& Compiler::getCurrentChunk() {
        return chunk;
    }

    void Compiler::number() {
        auto& token = parser.getPreviousToken();
        double value = strtod(token.token.begin(), nullptr);
        emitConstant(value);
    }

    void Compiler::emitConstant(Value value) {
        try {
            getCurrentChunk().writeConstant(value, previousLine());
        } catch (lox::Exception& e) {
            parser.errorAtPrevious(e.what().c_str());
        }
    }

    size_t Compiler::previousLine() const {
        return parser.getPreviousToken().line;
    }

    void Compiler::expression() {
        parsePrecedence(Precedence::Assignment);
    }

    void Compiler::grouping() {
        expression();
        parser.consume(TokenType::RightParen, "Expect ')' after parentheses");
    }

    void Compiler::unary() {
        Token previous = parser.getPreviousToken();

        parsePrecedence(Precedence::Unary);
        switch (previous.type) {
        case TokenType::Minus:
            emit(OpCode::Negate);
            break;
        default:
            std::unreachable();
        }
    }

    Precedence nextPrecedence(Precedence p) {
        return Precedence{std::to_underlying(p) + 1};
    }

    void Compiler::binary() {
        TokenType operatorType = parser.getPreviousToken().type;
        auto rule = getRule(operatorType);
        parsePrecedence(nextPrecedence(rule.precedence));
        switch (operatorType) {
        case TokenType::Plus:
            emit(OpCode::Add);
            break;
        case TokenType::Minus:
            emit(OpCode::Subtract);
            break;
        case TokenType::Star:
            emit(OpCode::Multiply);
            break;
        case TokenType::Slash:
            emit(OpCode::Divide);
            break;
        default:
            std::unreachable();
        }
    }

    void Compiler::ternary() {
        expression();
        parser.consume(TokenType::Colon, "Expected colon between ternary expressions");
        expression();
        // this will emit the code for expressions, but what do we do to jump between them?
    }

    const Compiler::ParseRule& Compiler::getRule(TokenType type) const {
        const static ParseRule empty{};
        const static std::unordered_map<TokenType, ParseRule> rules{
            {TokenType::LeftParen, {&Compiler::grouping}},
            {TokenType::Minus, {&Compiler::unary, &Compiler::binary, Precedence::Term}},
            {TokenType::Plus, {{}, &Compiler::binary, Precedence::Term}},
            {TokenType::Slash, {{}, &Compiler::binary, Precedence::Factor}},
            {TokenType::Star, {{}, &Compiler::binary, Precedence::Factor}},
            {TokenType::Number, {&Compiler::number}},
            {TokenType::Question, {{}, &Compiler::ternary, Precedence::Ternary}}};

        auto iter = rules.find(type);
        return iter == rules.end() ? empty : iter->second;
    }

    void Compiler::parsePrecedence(Precedence precedence) {
        auto& previous = parser.advance();
        auto prefixRule = getRule(previous.type).prefix;
        if (!prefixRule) {
            parser.errorAtPrevious("Expect expression.");
            return;
        }
        std::invoke(prefixRule, this);

        while (precedence <= getRule(parser.getCurrentToken().type).precedence) {
            previous = parser.advance();
            auto infixRule = getRule(previous.type).infix;
            std::invoke(infixRule, this);
        }
    }
}