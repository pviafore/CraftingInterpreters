#include "parser.h"

#include "print"
namespace lox {
    Parser::Parser(TokenIterator token) : current(token) {
    }
    void Parser::errorAtCurrent(StringView message) {
        printError(*current, message);
    }

    void Parser::errorAtPrevious(StringView message) {
        printError(previous, message);
    }

    void Parser::printError(const Token& token, StringView message) {
        if (panicMode) {
            return;
        }
        std::print(std::cerr, "[ line {} ] Error", token.line);
        if (token.type == TokenType::Eof) {
            std::print(std::cerr, " at end");
        } else {
            std::print(std::cerr, " at {}", token.token);
        }

        std::println(std::cerr, ": {}", message);
        error = true;
        panicMode = true;
    }

    bool Parser::hasError() const {
        return error;
    }

    Token& Parser::advance() {
        previous = *current;
        do {
            current++;
            if (current->type == TokenType::Error) {
                errorAtCurrent(current->token);
            }

        } while (current->type == TokenType::Error);
        return previous;
    }

    void Parser::consume(TokenType type, StringView message) {
        if (current->type == type) {
            advance();
            return;
        }
        errorAtCurrent(message);
    }

    const Token& Parser::getPreviousToken() const {
        return previous;
    }

    const Token& Parser::getCurrentToken() {
        return *current;
    }

    bool Parser::match(TokenType type) {
        if (!check(type)) {
            return false;
        }
        advance();
        return true;
    }

    bool Parser::check(TokenType type) {
        return current->type == type;
    }

    bool Parser::inPanicMode() const {
        return panicMode;
    }

    void Parser::synchronize() {
        panicMode = false;
        while (current->type != TokenType::Eof) {
            if (previous.type == TokenType::Semicolon) {
                return;
            }
            switch (current->type) {
            case TokenType::Class:
            case TokenType::Fun:
            case TokenType::Var:
            case TokenType::For:
            case TokenType::If:
            case TokenType::While:
            case TokenType::Print:
            case TokenType::Return:
                return;
            default:;  // do nothing
            }
            advance();
        }
    }

}