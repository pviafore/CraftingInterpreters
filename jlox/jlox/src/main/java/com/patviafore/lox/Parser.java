package com.patviafore.lox;

import java.util.ArrayList;
import java.util.List;

public class Parser {

    private static class ParseError extends RuntimeException {}

    private final List<Token> tokens;
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }

    List<Stmt> parse() {
        List<Stmt> statements = new ArrayList<>();
        while(!isAtEnd()) {
            statements.add(declaration());
        }
        return statements;
    }

    Expr parseExpression() { 
        try{
            return expression();
        }
        catch(ParseError error) {
            return null;
        }
    }

    private Stmt declaration() {
        try {
            if(match(TokenType.VAR)) return varDeclaration();
            return statement();
        }
        catch (ParseError error){
            synchronize();
            return null;
        }
    }

    private Stmt varDeclaration() {
        Token name = consume(TokenType.IDENTIFIER, "Expect variable name");
        Expr initializer = null;
        if(match(TokenType.EQUAL)) {
            initializer = expression();
        }
        //if(initializer == null){
         ////   initializer = new Uninitialized();
        //}
        consumeUntilSemicolon("variable declaration");
        return new Stmt.Var(name, initializer);
    }

    private Stmt statement() {
        if(match(TokenType.PRINT)) return printStatement();
        if(match(TokenType.LEFT_BRACE)) return new Stmt.Block(block());
        return expressionStatement();
    }

    private List<Stmt> block() {
        List<Stmt> statements = new ArrayList<>();
        while (!check(TokenType.RIGHT_BRACE) && !isAtEnd()) {
            statements.add(declaration());
        }
        consume(TokenType.RIGHT_BRACE, "Expect '} after block.");
        return statements;
    }

    private Stmt printStatement() {
        Expr value = expression();
        consumeUntilSemicolon();
        return new Stmt.Print(value);
    }

    private Stmt expressionStatement() {
        Expr expr = expression();
        consumeUntilSemicolon();
        return new Stmt.Expression(expr);
    }


    private void consumeUntilSemicolon(){
        consumeUntilSemicolon("value");
    }

    private void consumeUntilSemicolon(String statementType) {
        consume(TokenType.SEMICOLON, "Expect ';' after "+statementType+".");
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
        return binaryParse(() -> assignment(), TokenType.COMMA);
    }

    private Expr assignment() {
        Expr expr = ternary();

        if(match(TokenType.EQUAL)){
            Token equals = previous();
            Expr value = assignment();
            
            if(expr instanceof Expr.Variable) {
                Token name = ((Expr.Variable)expr).name;
                return new Expr.Assign(name, value);
            }

            error(equals, "Invalid assignment target.");
        }
        return expr;

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

        if(match(TokenType.IDENTIFIER)) {
            return new Expr.Variable(previous());
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
