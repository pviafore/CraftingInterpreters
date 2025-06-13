#include "parser.h"

#include "print"
namespace lox {
    Parser::Parser(TokenIterator token) : current(token) {
    }
    void Parser::errorAtCurrent(const String& message) {
        printError(*current, message);
    }

    void Parser::errorAtPrevious(const String& message) {
        printError(previous, message);
    }

    void Parser::printError(const Token& token, const String& message) {
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

    void Parser::consume(TokenType type, const String& message) {
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
}