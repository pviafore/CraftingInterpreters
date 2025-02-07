#ifndef _LOXCPP_PARSER_H_
#define _LOXCPP_PARSER_H_

#include "scanner.h"
namespace lox {
    class Parser {
    public:
        Parser(TokenIterator token);
        void errorAtCurrent(const String& message);
        void errorAtPrevious(const String& message);
        bool hasError() const;
        void consume(TokenType, const String& message);
        const Token& getPreviousToken() const;
        const Token& getCurrentToken();
        Token& advance();

    private:
        void printError(const Token& t, const String& message);
        TokenIterator current;
        Token previous;
        bool error = false;
        bool panicMode = false;
    };
}

#endif