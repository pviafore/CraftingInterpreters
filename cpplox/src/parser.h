#ifndef _LOXCPP_PARSER_H_
#define _LOXCPP_PARSER_H_

#include "scanner.h"
namespace lox {
    class Parser {
    public:
        Parser(TokenIterator token);
        void errorAtCurrent(StringView message);
        void errorAtPrevious(StringView message);
        bool hasError() const;
        void consume(TokenType, StringView message);
        const Token& getPreviousToken() const;
        const Token& getCurrentToken();
        Token& advance();
        bool match(TokenType type);
        bool check(TokenType type);
        bool inPanicMode() const;
        void synchronize();

    private:
        void printError(const Token& t, StringView message);
        TokenIterator current;
        Token previous;
        bool error = false;
        bool panicMode = false;
    };
}

#endif