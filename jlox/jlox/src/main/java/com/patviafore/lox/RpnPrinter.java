package com.patviafore.lox;

import com.patviafore.lox.Expr.Grouping;
import com.patviafore.lox.Expr.Literal;
import com.patviafore.lox.Expr.Unary;

public class RpnPrinter implements Expr.Visitor<String> {
    String print(Expr expr) {
        return expr.accept(this);
    }

    @Override 
    public String visitBinaryExpr(Expr.Binary expr) {
        return print(expr.left) + " " + print(expr.right) + " " + expr.operator.lexeme;
    }

    @Override
    public String visitGroupingExpr(Grouping expr) {
        return print(expr.expression) + " group" ;
    }

    @Override
    public String visitLiteralExpr(Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitUnaryExpr(Unary expr) {
        // to disambiguate from negation to subtraction
        if(expr.operator.lexeme == "-") {
            return parenthesize(expr.operator.lexeme, expr.right);
        }
        return print(expr.right) + " " + expr.operator.lexeme;
    }

    private String parenthesize(String name, Expr... exprs) {
        StringBuilder builder = new StringBuilder();
        builder.append("(");
        for(Expr expr: exprs) {
            builder.append(" ");
            builder.append(print(expr));
        }

        builder.append(" ").append(name);

        builder.append(")");

        return builder.toString();
    }

    public static void main(String[] args) {
        Expr expression = new Expr.Binary(
            new Expr.Unary(
                new Token(TokenType.MINUS, "-", null, 1),
                new Expr.Literal(123)
            ),
            new Token(TokenType.STAR, "*", null, 1),
            new Expr.Grouping(new Expr.Literal(45.67))
        );

        System.out.println(new RpnPrinter().print(expression));
    }

    
}
