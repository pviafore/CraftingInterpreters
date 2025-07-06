#include "scanner.h"

#include <cstring>
namespace lox {
    const char* errorMessage = "Unexpected character.";
    Scanner::Scanner(String s) : s(std::move(s)) {}

    TokenIterator Scanner::begin() const {
        return TokenIterator(s.begin());
    }

    TokenIterator Scanner::end() const {
        return TokenIterator(s.end());
    }

    TokenIterator::TokenIterator(const char* ptr) : start(ptr), ptr(ptr) {
    }
    Token& TokenIterator::operator*() {
        if (!parsed) {
            parseToken();
        }
        return token;
    }
    TokenIterator TokenIterator::operator++(int) {
        TokenIterator value = *this;
        parseToken();
        return value;
    }

    TokenIterator& TokenIterator::operator++() {
        parseToken();
        return *this;
    }
    Token* TokenIterator::operator->() {
        if (!parsed) {
            parseToken();
        }
        return &token;
    }

    void TokenIterator::parseToken() {
        skipWhitespace();
        start = ptr;
        if (isAtEnd()) {
            token = makeToken(TokenType::Eof);
            return;
        }
        parsed = true;
        token = scanToken();
        return;
    }

    Token TokenIterator::scanToken() {
        char c = advance();
        if (isalpha(c) || c == '_') {
            return identifier();
        }
        if (isdigit(c)) {
            return number();
        }
        switch (c) {
        case ')':
            return makeToken(TokenType::RightParen);
        case '(':
            return makeToken(TokenType::LeftParen);
        case '{':
            return makeToken(TokenType::LeftBrace);
        case '}':
            return makeToken(TokenType::RightBrace);
        case ';':
            return makeToken(TokenType::Semicolon);
        case ':':
            return makeToken(TokenType::Colon);
        case ',':
            return makeToken(TokenType::Comma);
        case '.':
            return makeToken(TokenType::Dot);
        case '-':
            return makeToken(TokenType::Minus);
        case '+':
            return makeToken(TokenType::Plus);
        case '?':
            return makeToken(TokenType::Question);
        case '/':
            return makeToken(TokenType::Slash);
        case '*':
            return makeToken(TokenType::Star);
        case '!':
            return makeToken(match('=') ? TokenType::BangEqual : TokenType::Bang);
        case '=':
            return makeToken(match('=') ? TokenType::EqualEqual : TokenType::Equal);
        case '<':
            return makeToken(match('=') ? TokenType::LessEqual : TokenType::Less);
        case '>':
            return makeToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
        case '"':
            return string();
        }
        return errorToken(errorMessage);
    }

    bool TokenIterator::isAtEnd() const {
        return peek() == '\0';
    }

    Token TokenIterator::makeToken(TokenType type) const {
        return {type, line, {start, ptr}};
    }

    Token TokenIterator::errorToken(const char* message) const {
        return {TokenType::Error, line, {message, strlen(message)}};
    }

    char TokenIterator::advance() {
        return *ptr++;
    }

    bool TokenIterator::match(char expected) {
        if (isAtEnd()) {
            return false;
        }
        if (peek() != expected) {
            return false;
        }
        ++ptr;
        return true;
    }

    void TokenIterator::skipWhitespace() {
        while (true) {
            char c = peek();
            switch (c) {
            case '\n':
                ++line;
                advance();
                break;
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
            default:
                return;
            }
        }
    }

    char TokenIterator::peek() const {
        return *ptr;
    }

    char TokenIterator::peekNext() const {
        if (isAtEnd()) {
            return '\0';
        }
        return *(ptr + 1);
    }

    Token TokenIterator::string() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') {
                ++line;
            }
            advance();
        }
        if (isAtEnd()) {
            return errorToken("Unterminated string.");
        }

        advance();
        return makeToken(TokenType::String);
    }

    Token TokenIterator::number() {
        while (isdigit(peek())) {
            advance();
        }

        if (peek() == '.' && isdigit(peekNext())) {
            advance();
        }

        while (isdigit(peek())) {
            advance();
        }
        return makeToken(TokenType::Number);
    }

    Token TokenIterator::identifier() {
        while (isalnum(peek()) || peek() == '_') {
            advance();
        }
        return makeToken(identifierType());
    }

    TokenType TokenIterator::identifierType() const {
        switch (*start) {
        case 'a':
            return checkKeyword(1, "nd", TokenType::And);
        case 'c':
            if (ptr - start > 1) {
                switch (*(start + 1)) {
                case 'l':
                    return checkKeyword(2, "ass", TokenType::Class);
                case 'o':
                    return checkKeyword(2, "nst", TokenType::Const);
                }
            }
            break;
        case 'e':
            return checkKeyword(1, "lse", TokenType::Else);
        case 'i':
            return checkKeyword(1, "f", TokenType::If);
        case 'n':
            return checkKeyword(1, "il", TokenType::Nil);
        case 'o':
            return checkKeyword(1, "r", TokenType::Or);
        case 'p':
            return checkKeyword(1, "rint", TokenType::Print);
        case 'r':
            return checkKeyword(1, "eturn", TokenType::Return);
        case 's':
            return checkKeyword(1, "uper", TokenType::Super);
        case 'v':
            return checkKeyword(1, "ar", TokenType::Var);
        case 'w':
            return checkKeyword(1, "hile", TokenType::While);
        case 'f':
            if (ptr - start > 1) {
                switch (*(start + 1)) {
                case 'a':
                    return checkKeyword(2, "lse", TokenType::False);
                case 'o':
                    return checkKeyword(2, "r", TokenType::For);
                case 'u':
                    return checkKeyword(2, "n", TokenType::Fun);
                }
            }
            break;
        case 't':
            if (ptr - start > 1) {
                switch (*(start + 1)) {
                case 'h':
                    return checkKeyword(2, "is", TokenType::This);
                case 'r':
                    return checkKeyword(2, "ue", TokenType::True);
                }
            }
            break;
        }
        return TokenType::Identifier;
    }

    TokenType TokenIterator::checkKeyword(size_t offset, String s, TokenType type) const {
        if (static_cast<size_t>(ptr - start) == offset + s.size() && s == String(start + offset, s.size())) {
            return type;
        }
        return TokenType::Identifier;
    }
}