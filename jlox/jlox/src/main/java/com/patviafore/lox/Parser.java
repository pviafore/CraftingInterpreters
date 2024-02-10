package com.patviafore.lox;

import java.util.List;
import java.util.Optional;

public class Parser {

    private static class ParseError extends RuntimeException {}

    private final List<Token> tokens;
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }

    Optional<Expr> parse() {
        try {
            return Optional.of(expression());
        }
        catch (ParseError error) {
            return Optional.empty();
        }
    }

    private Expr expression() {
        return comma();
    }

    private interface ExprBinaryParse {
        public Expr parse();
    }

    private Expr binaryParse(ExprBinaryParse parse, TokenType... tokens){
        Expr expr = parse.parse();
        while(match(tokens)) {
            Token operator = previous();
            Expr right = parse.parse();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;

    }

    private Expr comma() { 
        return binaryParse(() -> ternary(), TokenType.COMMA);
    }

    private Expr ternary() {
        Expr condition = equality();
        while(match(TokenType.QUESTION)) {
            Token operator = previous();
            Expr left = equality();
            if(!match(TokenType.COLON)) {
                throw error(peek(), "Expected : during ternary");
            }
            Expr right = equality();
            condition = new Expr.Ternary(condition, operator, left, right);
        }

        return condition;

    }
    private Expr equality() {
        return binaryParse(() -> comparison(), TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL);
    }

    private Expr comparison() {
        return binaryParse(() -> term(), TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL);
    }

    private Expr term() {
        return binaryParse(() -> factor(), TokenType.MINUS, TokenType.PLUS);
    }

    private Expr factor() {
        return binaryParse(() -> unary(), TokenType.SLASH, TokenType.STAR);
    }

    private Expr unary() {
        if(match(TokenType.BANG, TokenType.MINUS)){
            Token operator = previous();
            Expr right = unary();
            return new Expr.Unary(operator, right);
        }

        return primary();
    }

    private Expr primary() {
        if(match(TokenType.FALSE)) return new Expr.Literal(false);
        if(match(TokenType.TRUE)) return new Expr.Literal(true);
        if(match(TokenType.NIL)) return new Expr.Literal(null);

        if(match(TokenType.NUMBER, TokenType.STRING)){
            return new Expr.Literal(previous().literal);
        }

        if(match(TokenType.LEFT_PAREN)){
            Expr expr = expression();
            consume(TokenType.RIGHT_PAREN, "Expect ')' after expression.");
            return new Expr.Grouping(expr);
        }

        throw error(peek(), "Expect expression.");
    }

    private boolean match(TokenType... types) {
        for (TokenType type : types ) {
            if(check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    private boolean check(TokenType type) {
        if(isAtEnd()) return false;
        return peek().type == type;
    }

    private Token advance() {
        if(!isAtEnd()) current++;
        return previous();
    }

    private boolean isAtEnd() { 
        return peek().type == TokenType.EOF;
    }

    private Token peek() {
        return tokens.get(current);
    }

    private Token previous() {
        return tokens.get(current -1);
    }

    private Token consume(TokenType type, String message) {
        if(check(type)) return advance();
        throw error(peek(), message);
    }

    private ParseError error(Token token, String message) {
        Lox.error(token, message);
        return new ParseError();
    }

    private void synchronize() {
        advance();
        while(!isAtEnd()) {
            if(previous().type == TokenType.SEMICOLON) return;

            switch(peek().type){
                case CLASS:
                case FOR:
                case FUN:
                case IF:
                case PRINT:
                case RETURN:
                case VAR:
                case WHILE:
                    return;
                default: break;
            }

            advance();
        }
    }

}
