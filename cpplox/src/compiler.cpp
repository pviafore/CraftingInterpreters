#include "compiler.h"

#include <print>

#include "algorithm.h"
#include "debug.h"
#include "span.h"

namespace lox {
    namespace ReservedInternal {
        const StringView SwitchCondition = "1__SwitchCondition__";
        const StringView OnceTracker = "2__OnceTracker__";
        const StringView ProgramState = "3__ProgramState__";
        const StringView ForStatement = "4__ForStatement";
    }

    Compiler::Compiler(const String& s) : scanner(s), parser(SharedPtr<Parser>::Make(scanner.begin())), function(SharedPtr<Function>::Make("<script>")), functionType(FunctionType::SCRIPT) {}
    Compiler::Compiler(Compiler* enclosing, FunctionType type) : debugMode(enclosing->debugMode), scanner(nullptr), parser(enclosing->parser), depth(enclosing->depth + 1), function(SharedPtr<Function>::Make(parser->getPreviousToken().token)), functionType(type), enclosing(enclosing), classCompiler(enclosing->classCompiler), currentClass(enclosing->currentClass) {
    }
    void Compiler::beginCompile() {
        StringView name = (functionType != FunctionType::FUNCTION ? "this" : ReservedInternal::ProgramState);
        addLocal(name, false);  // the function that is being called pushed by VM
        markInitialized();
    }

    void Compiler::setupOnceTracking() {
        emitConstant(0.0);  // for onces
        onceTracker = addLocal(ReservedInternal::OnceTracker, false);
        markInitialized();
    }
    SharedPtr<Function> Compiler::compile() {
        beginCompile();
        setupOnceTracking();
        while (!parser->match(TokenType::Eof)) {
            declaration();
        }
        parser->consume(TokenType::Eof, "Expect end of expression");
        return endCompile();
    }

    SharedPtr<Function> Compiler::endCompile() {
        constants.clear();
        if (parser->hasError() || !function) {
            return nullptr;
        }

        function->getChunk()->write(OpCode::Pop, parser->getPreviousToken().line);  // for once tracker
        emitReturn();
        if (debugMode && !parser->hasError()) {
            std::println("{}\n{}", function->getName(), **(function->getChunk()));
        }

        return function;
    }

    void Compiler::emitReturn() {
        if (functionType == FunctionType::INITIALIZER) {
            getCurrentChunk()->writeOpAndIndex(OpCode::GetLocal, OpCode::GetLocal, 0, parser->getPreviousToken().line);
        } else {
            emit(OpCode::Nil);
        }
        function->getChunk()->write(OpCode::Return, parser->getPreviousToken().line);
    }

    void Compiler::emit(std::byte byte) {
        getCurrentChunk()->write(byte, parser->getPreviousToken().line);
    }

    void Compiler::emit(OpCode b1, std::byte b2) {
        emit(b1);
        emit(b2);
    }

    void Compiler::emit(OpCode b1) {
        emit(std::byte{std::to_underlying(b1)});
    }

    SharedPtr<Chunk> Compiler::getCurrentChunk() {
        return function->getChunk();
    }

    void Compiler::number(bool) {
        auto& token = parser->getPreviousToken();
        double value = strtod(token.token.begin(), nullptr);
        emitConstant(value);
    }

    void Compiler::string(bool) {
        auto token = parser->getPreviousToken().token;
        auto s = StringView(token.begin() + 1, token.end() - 1);
        emitConstant(s);
    }

    void Compiler::emitConstant(Value value) {
        try {
            getCurrentChunk()->writeConstant(value, previousLine());
        } catch (lox::Exception& e) {
            parser->errorAtPrevious(e.what().c_str());
        }
    }

    void Compiler::emit(size_t value) {
        return getCurrentChunk()->write(value, previousLine());
    }

    size_t Compiler::previousLine() const {
        return parser->getPreviousToken().line;
    }

    void Compiler::expression() {
        parsePrecedence(Precedence::Assignment);
    }

    void Compiler::grouping(bool) {
        expression();
        parser->consume(TokenType::RightParen, "Expect ')' after parentheses");
    }

    void Compiler::call(bool) {
        uint8_t argCount = argumentList();
        emit(OpCode::Call);
        emit(argCount);
    }

    uint8_t Compiler::argumentList() {
        uint8_t argCount = 0;
        if (!parser->check(TokenType::RightParen)) {
            do {
                expression();
                if (argCount == 255) {
                    parser->errorAtPrevious("Cannot have more than 255 arguments");
                }
                argCount++;
            } while (parser->match(TokenType::Comma));
        }
        parser->consume(TokenType::RightParen, "Expect ')' after arguments.");
        return argCount;
    }

    void Compiler::unary(bool) {
        Token previous = parser->getPreviousToken();

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

    void Compiler::dot(bool canAssign) {
        parser->consume(TokenType::Identifier, "Expect property name after .");
        auto constant = addIdentifierConstant(parser->getPreviousToken().token, false);
        if (canAssign && parser->match(TokenType::Equal)) {
            expression();
            emit(OpCode::SetProperty);
            emit(constant);
        } else if (parser->match(TokenType::LeftParen)) {
            auto argCount = argumentList();
            emit(OpCode::Invoke);
            emit(constant);
            emit(argCount);
        } else {
            emit(OpCode::GetProperty);
            emit(constant);
        }
    }

    void Compiler::binary(bool) {
        TokenType operatorType = parser->getPreviousToken().type;
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

    void Compiler::andOp(bool) {
        auto endJump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);
        parsePrecedence(Precedence::And);
        patchJump(endJump);
    }

    void Compiler::orOp(bool) {
        auto elseJump = emitJump(OpCode::JumpIfFalse);
        auto endJump = emitJump(OpCode::Jump);

        patchJump(elseJump);
        emit(OpCode::Pop);
        parsePrecedence(Precedence::Or);

        patchJump(endJump);
    }

    void Compiler::ternary(bool) {
        expression();
        parser->consume(TokenType::Colon, "Expected colon between ternary expressions");
        expression();
        // this will emit the code for expressions, but what do we do to jump between them?
    }

    void Compiler::literal(bool) {
        switch (parser->getPreviousToken().type) {
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
            {TokenType::LeftParen, {&Compiler::grouping, &Compiler::call, Precedence::Call}},
            {TokenType::Dot, {nullptr, &Compiler::dot, Precedence::Call}},
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
            {TokenType::And, {nullptr, &Compiler::andOp, Precedence::And}},
            {TokenType::Or, {nullptr, &Compiler::orOp, Precedence::Or}},
            {TokenType::Identifier, {&Compiler::variable}},
            {TokenType::Number, {&Compiler::number}},
            {TokenType::String, {&Compiler::string}},
            {TokenType::Super, {&Compiler::super_}},
            {TokenType::False, {&Compiler::literal}},
            {TokenType::True, {&Compiler::literal}},
            {TokenType::Nil, {&Compiler::literal}},
            {TokenType::This, {&Compiler::this_}},
            {TokenType::Question, {{}, &Compiler::ternary, Precedence::Ternary}}};

        auto iter = rules.find(type);
        return iter == rules.end() ? empty : iter->second;
    }

    void Compiler::parsePrecedence(Precedence precedence) {
        auto& previous = parser->advance();
        auto prefixRule = getRule(previous.type).prefix;
        if (!prefixRule) {
            parser->errorAtPrevious("Expect expression.");
            return;
        }
        bool canAssign = precedence <= Precedence::Assignment;
        std::invoke(prefixRule, this, canAssign);

        while (precedence <= getRule(parser->getCurrentToken().type).precedence) {
            previous = parser->advance();
            auto infixRule = getRule(previous.type).infix;
            std::invoke(infixRule, this, canAssign);
        }

        if (canAssign && parser->match(TokenType::Equal)) {
            parser->errorAtPrevious("Invalid assignment target");
        }
    }

    void Compiler::declaration() {
        if (parser->match(TokenType::Var)) {
            varDeclaration();
        } else if (parser->match(TokenType::Const)) {
            constDeclaration();
        } else if (parser->match(TokenType::Fun)) {
            funDeclaration();
        } else if (parser->match(TokenType::Class)) {
            classDeclaration();
        } else {
            statement();
        }

        if (parser->inPanicMode()) {
            parser->synchronize();
        }
    }

    void Compiler::statement() {
        if (parser->match(TokenType::Print)) {
            printStatement();
        } else if (parser->match(TokenType::If)) {
            ifStatement();
        } else if (parser->match(TokenType::While)) {
            whileStatement();
        } else if (parser->match(TokenType::For)) {
            forStatement();
        } else if (parser->match(TokenType::Switch)) {
            switchStatement();
        } else if (parser->match(TokenType::Break)) {
            breakStatement();
        } else if (parser->match(TokenType::Continue)) {
            continueStatement();
        } else if (parser->match(TokenType::Once)) {
            onceStatement();
        } else if (parser->match(TokenType::Return)) {
            returnStatement();
        } else if (parser->match(TokenType::LeftBrace)) {
            beginScope();
            block();
            endScope();
        } else {
            expressionStatement();
        }
    }

    void Compiler::block() {
        while (!parser->check(TokenType::RightBrace) && !parser->check(TokenType::Eof)) {
            declaration();
        }
        parser->consume(TokenType::RightBrace, "Expect '}' after block.");
    }

    void Compiler::beginScope() {
        depth++;
    }
    void Compiler::endScope() {
        depth--;

        while (locals.size() > 0 && locals.back().depth && locals.back().depth.value() > depth) {
            auto opcode = (locals.back().isCaptured) ? OpCode::CloseUpValue : OpCode::Pop;
            emit(opcode);
            locals.pop_back();
        }
    }

    void Compiler::printStatement() {
        expression();
        parser->consume(TokenType::Semicolon, "Expect ';' after value");
        emit(OpCode::Print);
    }

    void Compiler::ifStatement() {
        parser->consume(TokenType::LeftParen, "Expect '(' after if )");
        expression();
        parser->consume(TokenType::RightParen, "Expect ')' after if condition )");
        int thenJump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);
        statement();
        int elseJump = emitJump(OpCode::Jump);
        patchJump(thenJump);
        emit(OpCode::Pop);
        if (parser->match(TokenType::Else)) {
            statement();
        }
        patchJump(elseJump);
    }

    void Compiler::whileStatement() {
        nestedLoops.push_back(Loop{depth, getCurrentChunk()->size()});
        auto loopStart = getCurrentChunk()->size();
        parser->consume(TokenType::LeftParen, "Expect '(' after while )");
        expression();
        parser->consume(TokenType::RightParen, "Expect ')' after while condition )");

        int exitJump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);
        statement();
        emitLoop(loopStart);

        patchJump(exitJump);
        emit(OpCode::Pop);
        for (auto jump : nestedLoops[nestedLoops.size() - 1].breakLocations) {
            patchJump(jump);
        }
        nestedLoops.pop_back();
    }

    void Compiler::forStatement() {
        beginScope();
        nestedLoops.push_back(Loop{depth});
        parser->consume(TokenType::LeftParen, "Expect (' after 'for'. ");
        StringView name;
        if (parser->match(TokenType::Semicolon)) {
            // no initializer
        } else if (parser->match(TokenType::Var)) {
            name = varDeclaration();
        } else {
            expressionStatement();
        }

        int loopStart = getCurrentChunk()->size();

        std::optional<size_t> exitJump = std::nullopt;
        if (!parser->match(TokenType::Semicolon)) {
            expression();
            parser->consume(TokenType::Semicolon, "Expect ';' after loop condition");
            exitJump = emitJump(OpCode::JumpIfFalse);
            emit(OpCode::Pop);
        }
        if (!parser->match(TokenType::RightParen)) {
            size_t bodyJump = emitJump(OpCode::Jump);
            size_t incrementStart = getCurrentChunk()->size();
            nestedLoops[nestedLoops.size() - 1].startLocation = incrementStart;
            expression();
            emit(OpCode::Pop);
            parser->consume(TokenType::RightParen, "Expect '}' after for clauses");
            emitLoop(loopStart);
            loopStart = incrementStart;
            patchJump(bodyJump);
        }
        Compiler compiler(this, functionType);
        compiler.beginScope();
        compiler.beginCompile();
        compiler.addLocal(name, true);
        compiler.markInitialized();
        compiler.setupOnceTracking();
        compiler.statement();
        auto f = compiler.endCompile();
        compiler.endScope();
        if (!f) {
            return;
        }
        f->setName(ReservedInternal::ForStatement);
        if (name.size()) {
            f->increaseArity();

            emit(OpCode::Closure);

            size_t index = this->function->getChunk()->addConstant(f);
            if (index >= 255) {
                parser->errorAtPrevious("Too many constants");
            }
            emit(uint8_t(index & 0xFF));
            for (auto upvalue : f->getUpvalues()) {
                emit(upvalue.isLocal ? 1 : 0);
                emit((std::byte)upvalue.index);
            }

            auto local = resolveLocal(name);
            getCurrentChunk()->writeOpAndIndex(OpCode::GetLocal, OpCode::GetLocal, local.value(), parser->getPreviousToken().line);

            emit(OpCode::Call);
            emit(1);
        } else {
            emitConstant(f);
            emit(OpCode::Call);
            emit(0);
        }
        emit(OpCode::Pop);
        emitLoop(loopStart);

        if (exitJump.has_value()) {
            patchJump(exitJump.value());
            emit(OpCode::Pop);
        }

        for (auto jump : nestedLoops[nestedLoops.size() - 1].breakLocations) {
            patchJump(jump);
        }
        nestedLoops.pop_back();
        endScope();
    }

    void Compiler::switchStatement() {
        beginScope();
        parser->consume(TokenType::LeftParen, "Expect '(' after 'switch'. ");
        expression();
        parser->consume(TokenType::RightParen, "Expect ')' after switch condition )");
        parser->consume(TokenType::LeftBrace, "Expect '{' after switch condition");
        auto index = addLocal(ReservedInternal::SwitchCondition, true);
        getCurrentChunk()->writeOpAndIndex(OpCode::SetLocal, OpCode::SetLocal, index, parser->getPreviousToken().line);
        lox::Optional<size_t> lastJump;
        lox::Vector<size_t> endOfCaseJumps;
        while (parser->match(TokenType::Case) || parser->match(TokenType::Default)) {
            if (lastJump.hasValue()) {
                emit(OpCode::Pop);
                patchJump(lastJump.value());
                lastJump = {};
            }

            if (parser->getPreviousToken().type == TokenType::Case) {
                expression();
                parser->consume(TokenType::Colon, "Expect colon after case statement");
                getCurrentChunk()->writeOpAndIndex(OpCode::GetLocal, OpCode::GetLocal, index, parser->getPreviousToken().line);
                emit(OpCode::Equal);
                lastJump = emitJump(OpCode::JumpIfFalse);
            } else {
                parser->consume(TokenType::Colon, "Expect colon after default statement");
            }

            statement();
            auto endOfCaseJump = emitJump(OpCode::Jump);
            endOfCaseJumps.push_back(endOfCaseJump);
        }

        if (lastJump.hasValue()) {
            emit(OpCode::Pop);
            patchJump(lastJump.value());
            lastJump = {};
        }

        for (auto jump : endOfCaseJumps) {
            patchJump(jump);
        }

        parser->consume(TokenType::RightBrace, "Expect '}' after switch cases");

        endScope();
    }

    void Compiler::breakStatement() {
        if (nestedLoops.size() == 0) {
            parser->errorAtPrevious("Break can only be used in a loop");
            return;
        }
        parser->consume(TokenType::Semicolon, "Expect ';' after value");

        int local = locals.size() - 1;
        while (local >= 0 && locals[local].depth.value() > nestedLoops[nestedLoops.size() - 1].depth) {
            emit(OpCode::Pop);
            local--;
        }
        auto jump = emitJump(OpCode::Jump);
        nestedLoops[nestedLoops.size() - 1].breakLocations.push_back(jump);
    }

    void Compiler::continueStatement() {
        if (nestedLoops.size() == 0) {
            parser->errorAtPrevious("Continue can only be used in a loop");
            return;
        }
        parser->consume(TokenType::Semicolon, "Expect ';' after value");
        int local = locals.size() - 1;
        while (local >= 0 && locals[local].depth.value() > nestedLoops[nestedLoops.size() - 1].depth) {
            emit(OpCode::Pop);
            local--;
        }

        auto startLoop = nestedLoops[nestedLoops.size() - 1].startLocation;
        emitLoop(startLoop);
    }

    void Compiler::onceStatement() {
        if (numberOfOnces == 64) {
            parser->errorAtPrevious("Only 64 once statements allowed");
            return;
        }
        numberOfOnces++;
        uint64_t mask = 1 << numberOfOnces;
        getCurrentChunk()->writeOpAndIndex(OpCode::GetLocal, OpCode::GetLocal, onceTracker, parser->getPreviousToken().line);
        emitConstant(double(mask));
        emit(OpCode::BitwiseAnd);
        emitConstant(double(0));
        emit(OpCode::Equal);
        auto jump = emitJump(OpCode::JumpIfFalse);
        emit(OpCode::Pop);
        getCurrentChunk()->writeOpAndIndex(OpCode::GetLocal, OpCode::GetLocal, onceTracker, parser->getPreviousToken().line);
        emitConstant(double(mask));
        emit(OpCode::BitwiseOr);
        getCurrentChunk()->writeOpAndIndex(OpCode::SetLocal, OpCode::SetLocal, onceTracker, parser->getPreviousToken().line);
        emit(OpCode::Pop);
        statement();
        auto thenJump = emitJump(OpCode::Jump);
        patchJump(jump);
        emit(OpCode::Pop);
        patchJump(thenJump);
    }

    void Compiler::returnStatement() {
        if (functionType == FunctionType::SCRIPT) {
            parser->errorAtPrevious("Cannot return from top-level code");
            return;
        }
        if (parser->match(TokenType::Semicolon)) {
            emitReturn();
        } else {
            if (functionType == FunctionType::INITIALIZER) {
                parser->errorAtPrevious("Cannot return a value from an initializer");
                return;
            }
            expression();
            parser->consume(TokenType::Semicolon, "Expected semicolon after return value");
            emit(OpCode::Return);
        }
    }

    void Compiler::emitLoop(size_t pos) {
        size_t offset = getCurrentChunk()->size() - pos;
        emit(OpCode::Loop);

        if (offset > std::numeric_limits<uint16_t>::max()) {
            parser->errorAtPrevious("Loop body too large.");
        }

        emit((std::byte)(offset >> 8));
        emit((std::byte)(offset));
    }

    size_t Compiler::emitJump(OpCode opcode) {
        emit(opcode);
        emit((std::byte)0xFF);
        emit((std::byte)0xFF);
        return getCurrentChunk()->size() - 2;
    }

    void Compiler::patchJump(size_t pos) {
        size_t jump = getCurrentChunk()->size() - pos + 1;

        if (jump > std::numeric_limits<uint16_t>::max()) {
            parser->errorAtPrevious("Too much code to jump over");
        }

        getCurrentChunk()->writeAt(pos, (std::byte)(jump >> 8));
        getCurrentChunk()->writeAt(pos + 1, (std::byte)jump);
    }

    void Compiler::expressionStatement() {
        expression();
        parser->consume(TokenType::Semicolon, "Expect ';' after expression");
        emit(OpCode::Pop);
    }

    void Compiler::variable(bool canAssign) {
        emitNamedVariable(parser->getPreviousToken().token, canAssign);
    }

    void Compiler::emitNamedVariable(StringView name, bool canAssign) {
        bool isConstant = immutables.contains(name);
        OpCode setop = OpCode::SetGlobal;
        OpCode setLongOp = OpCode::LongSetGlobal;
        OpCode getop = OpCode::GetGlobal;
        OpCode getLongOp = OpCode::LongGetGlobal;
        auto index = resolveLocal(name);
        if (index.hasValue()) {
            setop = setLongOp = OpCode::SetLocal;
            getop = getLongOp = OpCode::GetLocal;
            isConstant = locals[index.value()].constant;
        } else if (index = resolveUpvalue(name); index.hasValue()) {
            setop = setLongOp = OpCode::SetUpValue;
            getop = getLongOp = OpCode::GetUpValue;
        } else {
            index = addIdentifierConstant(name, isConstant);
        }

        if (canAssign && parser->match(TokenType::Equal)) {
            if (isConstant) {
                parser->errorAtPrevious("Can't re-assign constant");
                return;
            }
            expression();
            getCurrentChunk()->writeOpAndIndex(setop, setLongOp, index.value(), parser->getPreviousToken().line);
        } else {
            getCurrentChunk()->writeOpAndIndex(getop, getLongOp, index.value(), parser->getPreviousToken().line);
        }
    }

    Optional<size_t> Compiler::resolveLocal(StringView name) {
        auto newName = manglePrivate(name);
        for (int index = locals.size() - 1; index >= 0; --index) {
            Local& l = locals[index];
            if (StringView(newName) == l.name) {
                if (!l.depth.hasValue()) {
                    parser->errorAtPrevious("Can't read local variable in own initializer");
                }
                return index;
            }
        }
        return {};
    }

    Optional<size_t> Compiler::resolveUpvalue(StringView name) {
        if (enclosing) {
            auto local = enclosing->resolveLocal(name);
            if (local.hasValue()) {
                enclosing->locals[local.value()].isCaptured = true;
                return addUpvalue(local.value(), true);
            }

            auto upvalue = enclosing->resolveUpvalue(name);
            if (upvalue.hasValue()) {
                return addUpvalue(upvalue.value(), false);
            }
        }
        return {};
    }

    size_t Compiler::addUpvalue(size_t index, bool isLocal) {
        auto upvalueIndex = function->getUpvalue(index, isLocal);
        if (upvalueIndex.hasValue()) {
            return index;
        }
        return function->addUpvalue(index, isLocal);
    }

    StringView Compiler::varDeclaration(bool constant) {
        auto global = parseVariable("Expect variable name", constant);
        auto name = parser->getPreviousToken();
        if (parser->match(TokenType::Equal)) {
            expression();
        } else {
            emit(OpCode::Nil);
        }

        parser->consume(TokenType::Semicolon, "Expect ';' after variable declaration.");
        defineVariable(global);
        return name.token;
    }

    void Compiler::constDeclaration() {
        varDeclaration(true);
    }

    void Compiler::classDeclaration() {
        parser->consume(TokenType::Identifier, "Expect class name.");
        auto token = parser->getPreviousToken();
        auto oldClass = currentClass;
        currentClass = token.token;
        auto constant = addIdentifierConstant(token.token, true);
        declareVariable(true);

        emit(OpCode::Class);
        emit(constant);
        defineVariable(constant);

        ClassCompiler compiler;
        compiler.enclosing = classCompiler;
        classCompiler = &compiler;

        if (parser->match(TokenType::Less)) {
            parser->consume(TokenType::Identifier, "Expect superclass name.");
            variable(false);
            if (token.token == parser->getPreviousToken().token) {
                parser->errorAtPrevious("A class can't inherit from itself");
            }
            beginScope();
            auto place = addLocal("super", true);
            defineVariable(place);

            emitNamedVariable(token.token, false);
            emit(OpCode::Inherit);
            classCompiler->hasSuperclass = true;
        }
        emitNamedVariable(token.token, false);

        parser->consume(TokenType::LeftBrace, "Expect '{' before class body}");
        while (!parser->check(TokenType::RightBrace) && !parser->check(TokenType::Eof)) {
            method();
        }
        parser->consume(TokenType::RightBrace, "Expect '}' after class body}");
        emit(OpCode::Pop);
        if (classCompiler->hasSuperclass) {
            endScope();
        }
        classCompiler = classCompiler->enclosing;
        currentClass = oldClass;
    }

    void Compiler::method() {
        parser->consume(TokenType::Identifier, "Expect method name");
        auto constant = addIdentifierConstant(parser->getPreviousToken().token, true);
        FunctionType type = FunctionType::METHOD;
        if (parser->getPreviousToken().token == "init") {
            type = FunctionType::INITIALIZER;
        }

        func(type);
        emit(type == FunctionType::METHOD ? OpCode::Method : OpCode::Initializer);
        emit(constant);
    }

    void Compiler::funDeclaration() {
        size_t global = parseVariable("Expect function name. ", false);
        markInitialized();
        func(FunctionType::FUNCTION);
        defineVariable(global);
    }

    void Compiler::func(FunctionType type) {
        Compiler compiler(this, type);
        compiler.debugMode = debugMode;
        compiler.beginCompile();
        compiler.beginScope();
        parser->consume(TokenType::LeftParen, "Expect '(' after function name");
        if (!parser->check(TokenType::RightParen)) {
            do {
                compiler.function->increaseArity();
                size_t constant = compiler.parseVariable("Expect parameter name", true);
                compiler.defineVariable(constant);
            } while (parser->match(TokenType::Comma));
        }
        parser->consume(TokenType::RightParen, "Expect ')' after parameters");
        parser->consume(TokenType::LeftBrace, "Expect '{' before function body");
        compiler.setupOnceTracking();
        compiler.block();

        auto function = compiler.endCompile();
        if (!function) {
            return;
        }
        if (function->getUpValueCount()) {
            emit(OpCode::Closure);

            size_t index = this->function->getChunk()->addConstant(function);
            if (index >= 255) {
                parser->errorAtPrevious("Too many constants");
            }
            emit(uint8_t(index & 0xFF));
        } else {
            emitConstant(function);
        }
        for (auto& upvalue : function->getUpvalues()) {
            emit(upvalue.isLocal ? 1 : 0);
            emit(upvalue.index);
        }
    }

    size_t Compiler::parseVariable(StringView errorMessage, bool constant) {
        parser->consume(TokenType::Identifier, errorMessage);
        declareVariable(constant);
        if (depth > 0)
            return 0;  // don't do a global if we are a local
        return addIdentifierConstant(parser->getPreviousToken().token, constant);
    }

    size_t Compiler::addIdentifierConstant(StringView name, bool isConstant) {
        auto newName = manglePrivate(name);
        if (!constants.get(newName).hasValue()) {
            constants.insert(newName, getCurrentChunk()->addConstant(newName));
        }
        if (isConstant) {
            immutables.insert(newName);
        }
        return constants.get(newName).value();
    }

    void Compiler::defineVariable(size_t global) {
        if (depth > 0) {
            markInitialized();
            return;
        }
        getCurrentChunk()->writeOpAndIndex(OpCode::DefineGlobal, OpCode::LongDefineGlobal, global, parser->getPreviousToken().line);
    }

    void Compiler::markInitialized() {
        if (depth == 0) {
            return;
        }
        locals.back().depth = depth;
    }

    void Compiler::declareVariable(bool constant) {
        if (depth == 0) {
            return;
        }

        for (auto l : views::reversed<decltype(locals)>(locals)) {
            if (l.depth.hasValue() && l.depth.value() < depth) {
                break;
            }

            if (l.name == parser->getPreviousToken().token) {
                parser->errorAtPrevious("Already a variable with this name in scope");
            }
        }

        addLocal(parser->getPreviousToken().token, constant);
    }

    size_t Compiler::addLocal(StringView name, bool constant) {
        auto newName = manglePrivate(name);
        try {
            locals.push_back({newName, {}, constant});
        } catch (std::runtime_error&) {
            parser->errorAtCurrent("Too many local variables in function");
        }
        if (debugMode) {
            std::println("{} is added at {}", name, locals.size() - 1);
        }
        return locals.size() - 1;
    }

    void Compiler::this_(bool) {
        if (!classCompiler) {
            parser->errorAtPrevious("Can't use 'this' outside of a class");
            return;
        }
        variable(false);
    }

    void Compiler::super_(bool) {
        if (!classCompiler) {
            parser->errorAtPrevious("Cant use 'super' outside of class");
            return;
        } else if (!classCompiler->hasSuperclass) {
            parser->errorAtPrevious("Can't user 'super' in a class with no superclass");
            return;
        }
        parser->consume(TokenType::Dot, "Expect '.' afer 'super'.");
        parser->consume(TokenType::Identifier, "Expect superclass method name.");
        auto name = addIdentifierConstant(parser->getPreviousToken().token, false);
        emitNamedVariable("this", false);
        if (parser->match(TokenType::LeftParen)) {
            uint8_t argCount = argumentList();
            emitNamedVariable("super", false);
            emit(OpCode::SuperInvoke);
            emit(OpCode(name));
            emit(argCount);
        } else {
            emitNamedVariable("super", false);
            emit(OpCode::GetSuper);
            emit(name);
        }
    }

    String Compiler::manglePrivate(StringView name) {
        if (currentClass.size()) {
            if (name.size() >= 2 && name.begin()[0] == '_' && name.begin()[1] == '_') {
                return String("__") + currentClass + "_" + name;
            }
        }
        return name;
    }
}