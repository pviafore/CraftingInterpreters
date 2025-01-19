package com.patviafore.lox;

public enum TokenType {
    // Single Character Tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    QUESTION, COLON,

    // One or two characters
    BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, GREATER,
    GREATER_EQUAL, LESS, LESS_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    AND, BREAK, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR, PRINT,
    RETURN, INNER, THIS, TRUE, VAR, WHILE,

    EOF
}