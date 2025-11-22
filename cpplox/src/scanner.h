#ifndef LOXCPP_SCANNER_H_
#define LOXCPP_SCANNER_H_

#include <format>
#include <utility>

#include "interned.h"
#include "string.h"
namespace lox {

    enum class TokenType {
        // single character tokens
        LeftParen,
        RightParen,
        LeftBrace,
        RightBrace,
        Colon,
        Comma,
        Dot,
        Minus,
        Plus,
        Question,
        Semicolon,
        Slash,
        Star,

        // one or two characters
        Bang,
        BangEqual,
        Equal,
        EqualEqual,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
        // literals
        Identifier,
        String,
        Number,
        // keywords
        And,
        Break,
        Case,
        Class,
        Continue,
        Default,
        Const,
        Else,
        False,
        For,
        Fun,
        If,
        Nil,
        Once,
        Or,
        Print,
        Return,
        Super,
        Switch,
        This,
        True,
        Var,
        While,
        // other
        Error,
        Eof
    };

    struct Token {
        TokenType type = TokenType::Error;
        size_t line = 0;
        StringView token;
    };

    class TokenIterator {
    public:
        TokenIterator(const char* ptr);
        Token& operator*();
        TokenIterator operator++(int);
        TokenIterator& operator++();
        friend bool operator!=(const TokenIterator& ti1, const TokenIterator& ti2) {
            return ti1.ptr != ti2.ptr;
        }
        Token* operator->();
        const Token* operator->() const;

    private:
        void parseToken();
        Token scanToken();
        bool isAtEnd() const;
        Token makeToken(TokenType type) const;
        Token errorToken(const char* message) const;
        char advance();
        bool match(char expected);
        void skipWhitespace();
        char peek() const;
        char peekNext() const;
        Token string();
        Token number();
        Token identifier();
        TokenType identifierType() const;
        TokenType checkKeyword(size_t start, String s, TokenType type) const;
        Token token;
        const char* start;
        const char* ptr;
        bool parsed = false;
        size_t line = 1;
    };

    class Scanner {
    public:
        Scanner(String s);

        TokenIterator begin() const;
        TokenIterator end() const;

    private:
        InternedString s;
    };
}

template <>
struct std::formatter<lox::Token, char> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const lox::Token& token, FormatContext& ctx) const {
        std::format_to(ctx.out(), " {:2} {}", std::to_underlying(token.type), token.token);
        return ctx.out();
    }
};
#endif