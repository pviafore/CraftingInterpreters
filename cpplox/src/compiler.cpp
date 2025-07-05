#include "compiler.h"

#include <print>

#include "algorithm.h"
#include "debug.h"
#include "span.h"

namespace lox {
    Compiler::Compiler(const String& s) : scanner(s), parser(scanner.begin()) {}
    Optional<Chunk> Compiler::compile() {
        while (!parser.match(TokenType::Eof)) {
            declaration();
        }
        parser.consume(TokenType::Eof, "Expect end of expression");
        auto outChunk = parser.hasError() ? Optional<Chunk>{} : Optional{chunk};
        constants.clear();
        if (outChunk) {
            outChunk.value().write(OpCode::Return, parser.getPreviousToken().line);
        }
        if (debugMode && !parser.hasError()) {
            std::println("{}", outChunk.value());
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

    void Compiler::number(bool) {
        auto& token = parser.getPreviousToken();
        double value = strtod(token.token.begin(), nullptr);
        emitConstant(value);
    }

    void Compiler::string(bool) {
        auto token = parser.getPreviousToken().token;
        auto s = StringView(token.begin() + 1, token.end() - 1);
        emitConstant(s);
    }

    void Compiler::emitConstant(Value value) {
        try {
            getCurrentChunk().writeConstant(value, previousLine());
        } catch (lox::Exception& e) {
            parser.errorAtPrevious(e.what().c_str());
        }
    }

    void Compiler::emit(size_t value) {
        return getCurrentChunk().write(value, previousLine());
    }

    size_t Compiler::previousLine() const {
        return parser.getPreviousToken().line;
    }

    void Compiler::expression() {
        parsePrecedence(Precedence::Assignment);
    }

    void Compiler::grouping(bool) {
        expression();
        parser.consume(TokenType::RightParen, "Expect ')' after parentheses");
    }

    void Compiler::unary(bool) {
        Token previous = parser.getPreviousToken();

        parsePrecedence(Precedence::Unary);
        switch (previous.type) {
        case TokenType::Bang:
            emit(OpCode::Not);
            break;
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

    void Compiler::binary(bool) {
        TokenType operatorType = parser.getPreviousToken().type;
        auto rule = getRule(operatorType);
        parsePrecedence(nextPrecedence(rule.precedence));
        switch (operatorType) {
        case TokenType::BangEqual:
            emit(OpCode::Equal);
            emit(OpCode::Not);
            break;
        case TokenType::EqualEqual:
            emit(OpCode::Equal);
            break;
        case TokenType::Greater:
            emit(OpCode::Greater);
            break;
        case TokenType::GreaterEqual:
            emit(OpCode::Less);
            emit(OpCode::Not);
            break;
        case TokenType::Less:
            emit(OpCode::Less);
            break;
        case TokenType::LessEqual:
            emit(OpCode::Greater);
            emit(OpCode::Not);
            break;
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

    void Compiler::ternary(bool) {
        expression();
        parser.consume(TokenType::Colon, "Expected colon between ternary expressions");
        expression();
        // this will emit the code for expressions, but what do we do to jump between them?
    }

    void Compiler::literal(bool) {
        switch (parser.getPreviousToken().type) {
        case TokenType::False:
            emit(OpCode::False);
            break;
        case TokenType::True:
            emit(OpCode::True);
            break;
        case TokenType::Nil:
            emit(OpCode::Nil);
            break;
        default:
            std::unreachable();
            break;
        }
    }

    const Compiler::ParseRule& Compiler::getRule(TokenType type) const {
        const static ParseRule empty{};
        const static std::unordered_map<TokenType, ParseRule> rules{
            {TokenType::LeftParen, {&Compiler::grouping}},
            {TokenType::Minus, {&Compiler::unary, &Compiler::binary, Precedence::Term}},
            {TokenType::Plus, {{}, &Compiler::binary, Precedence::Term}},
            {TokenType::Slash, {{}, &Compiler::binary, Precedence::Factor}},
            {TokenType::Star, {{}, &Compiler::binary, Precedence::Factor}},
            {TokenType::Bang, {&Compiler::unary}},
            {TokenType::BangEqual, {nullptr, &Compiler::binary, Precedence::Equality}},
            {TokenType::EqualEqual, {nullptr, &Compiler::binary, Precedence::Equality}},
            {TokenType::Greater, {nullptr, &Compiler::binary, Precedence::Comparison}},
            {TokenType::GreaterEqual, {nullptr, &Compiler::binary, Precedence::Comparison}},
            {TokenType::Less, {nullptr, &Compiler::binary, Precedence::Comparison}},
            {TokenType::LessEqual, {nullptr, &Compiler::binary, Precedence::Comparison}},
            {TokenType::Identifier, {&Compiler::variable}},
            {TokenType::Number, {&Compiler::number}},
            {TokenType::String, {&Compiler::string}},
            {TokenType::False, {&Compiler::literal}},
            {TokenType::True, {&Compiler::literal}},
            {TokenType::Nil, {&Compiler::literal}},
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
        bool canAssign = precedence <= Precedence::Assignment;
        std::invoke(prefixRule, this, canAssign);

        while (precedence <= getRule(parser.getCurrentToken().type).precedence) {
            previous = parser.advance();
            auto infixRule = getRule(previous.type).infix;
            std::invoke(infixRule, this, canAssign);
        }

        if (canAssign && parser.match(TokenType::Equal)) {
            parser.errorAtPrevious("Invalid assignment target");
        }
    }

    void Compiler::declaration() {
        if (parser.match(TokenType::Var)) {
            varDeclaration();
        } else {
            statement();
        }

        if (parser.inPanicMode()) {
            parser.synchronize();
        }
    }

    void Compiler::statement() {
        if (parser.match(TokenType::Print)) {
            printStatement();
        } else if (parser.match(TokenType::LeftBrace)) {
            beginScope();
            block();
            endScope();
        } else {
            expressionStatement();
        }
    }

    void Compiler::block() {
        while (!parser.check(TokenType::RightBrace) && !parser.check(TokenType::Eof)) {
            declaration();
        }
        parser.consume(TokenType::RightBrace, "Expect '}' after block.");
    }

    void Compiler::beginScope() {
        depth++;
    }
    void Compiler::endScope() {
        depth--;

        while (locals.size() > 0 && locals.back().depth != depth) {
            emit(OpCode::Pop);
            locals.pop_back();
        }
    }

    void Compiler::printStatement() {
        expression();
        parser.consume(TokenType::Semicolon, "Expect ';' after value");
        emit(OpCode::Print);
    }

    void Compiler::expressionStatement() {
        expression();
        parser.consume(TokenType::Semicolon, "Expect ';' after expression");
        emit(OpCode::Pop);
    }

    void Compiler::variable(bool canAssign) {
        emitNamedVariable(parser.getPreviousToken().token, canAssign);
    }

    void Compiler::emitNamedVariable(StringView name, bool canAssign) {
        OpCode setop = OpCode::SetGlobal;
        OpCode setLongOp = OpCode::LongSetGlobal;
        OpCode getop = OpCode::GetGlobal;
        OpCode getLongOp = OpCode::LongGetGlobal;

        auto index = resolveLocal(name);
        if (index.hasValue()) {
            setop = setLongOp = OpCode::SetLocal;
            getop = getLongOp = OpCode::GetLocal;
        } else {
            index = addIdentifierConstant(name);
        }

        if (canAssign && parser.match(TokenType::Equal)) {
            expression();
            getCurrentChunk().writeOpAndIndex(setop, setLongOp, index.value(), parser.getPreviousToken().line);
        } else {
            getCurrentChunk().writeOpAndIndex(getop, getLongOp, index.value(), parser.getPreviousToken().line);
        }
    }

    Optional<size_t> Compiler::resolveLocal(StringView name) {
        for (int index = locals.size() - 1; index >= 0; --index) {
            Local& l = locals[index];
            if (name == l.name) {
                if (!l.depth.has_value()) {
                    parser.errorAtPrevious("Can't read local variable in own initializer");
                }
                return index;
            }
        }
        return {};
    }

    void Compiler::varDeclaration() {
        auto global = parseVariable("Expect variable name");

        if (parser.match(TokenType::Equal)) {
            expression();
        } else {
            emit(OpCode::Nil);
        }

        parser.consume(TokenType::Semicolon, "Expect ';' after variable declaration.");
        defineVariable(global);
    }

    size_t Compiler::parseVariable(StringView errorMessage) {
        parser.consume(TokenType::Identifier, errorMessage);
        declareVariable();
        if (depth > 0)
            return 0;  // don't do a global if we are a local
        return addIdentifierConstant(parser.getPreviousToken().token);
    }

    size_t Compiler::addIdentifierConstant(StringView name) {
        if (!constants.get(name).has_value()) {
            constants.insert(name, getCurrentChunk().addConstant(name));
        }
        return constants.get(name).value();
    }

    void Compiler::defineVariable(size_t global) {
        if (depth > 0) {
            markInitialized();
            return;
        }
        getCurrentChunk().writeOpAndIndex(OpCode::DefineGlobal, OpCode::LongDefineGlobal, global, parser.getPreviousToken().line);
    }

    void Compiler::markInitialized() {
        locals.back().depth = depth;
    }

    void Compiler::declareVariable() {
        if (depth == 0) {
            return;
        }

        for (auto l : views::reversed<decltype(locals)>(locals)) {
            if (l.depth.has_value() && l.depth.value() < depth) {
                break;
            }

            if (l.name == parser.getPreviousToken().token) {
                parser.errorAtPrevious("Already a variable with this name in scope");
            }
        }

        addLocal(parser.getPreviousToken().token);
    }

    void Compiler::addLocal(StringView name) {
        try {
            locals.push_back({name, {}});
        } catch (std::runtime_error&) {
            parser.errorAtCurrent("Too many local variables in function");
        }
    }
}