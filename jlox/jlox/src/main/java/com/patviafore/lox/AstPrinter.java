package com.patviafore.lox;

import java.util.stream.Collectors;

import com.patviafore.lox.Expr.Assign;
import com.patviafore.lox.Expr.Call;
import com.patviafore.lox.Expr.Closure;
import com.patviafore.lox.Expr.Get;
import com.patviafore.lox.Expr.Grouping;
import com.patviafore.lox.Expr.Literal;
import com.patviafore.lox.Expr.Logical;
import com.patviafore.lox.Expr.Set;
import com.patviafore.lox.Expr.Super;
import com.patviafore.lox.Expr.This;
import com.patviafore.lox.Expr.Unary;
import com.patviafore.lox.Expr.Variable;

public class AstPrinter implements Expr.Visitor<String> {
    String print(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public String visitTernaryExpr(Expr.Ternary expr) {
        return parenthesize(expr.operator.lexeme, expr.condition, expr.left, expr.right);
    }

    @Override 
    public String visitBinaryExpr(Expr.Binary expr) {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right);
    }

    @Override
    public String visitGroupingExpr(Grouping expr) {
        return parenthesize("group", expr.expression);
    }

    @Override
    public String visitLiteralExpr(Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitUnaryExpr(Unary expr) {
        return parenthesize(expr.operator.lexeme, expr.right);
    }

    private String parenthesize(String name, Expr... exprs) {
        StringBuilder builder = new StringBuilder();
        builder.append("(").append(name);
        for(Expr expr: exprs) {
            builder.append(" ");
            builder.append(expr.accept(this));
        }

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

        System.out.println(new AstPrinter().print(expression));
    }

    @Override
    public String visitVariableExpr(Variable expr) {
        return "var " + expr.name;
    }

    @Override
    public String visitAssignExpr(Assign expr) {
        return expr.name + " = " + print(expr.value);
    }

    @Override
    public String visitLogicalExpr(Logical expr) {
        return print(expr.left) + " " + expr.operator.lexeme + " " + print(expr.right);
    }

    @Override
    public String visitCallExpr(Call expr) {
        return print(expr.callee) + "(" + expr.arguments.stream().map((Expr e) -> print(e)).collect(Collectors.joining(",")) + ")";
    }
    
    @Override
    public String visitClosureExpr(Closure expr) {
        return "fun (" + expr.params.stream().map((Token t) -> t.lexeme).collect(Collectors.joining(",")) + ")";
    }

    @Override
    public String visitGetExpr(Get expr) {
        return "." + expr.name;
    }
    
    @Override
    public String visitSetExpr(Set expr) {
        return "." + expr.name + " = " + print(expr.value);
    }

    @Override
    public String visitThisExpr(This expr) {
        return "this";
    }
    
    @Override
    public String visitSuperExpr(Super expr) {
        return "super." + expr.method.lexeme;
    }
}