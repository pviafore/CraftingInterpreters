package com.patviafore.lox;

import java.util.List;

import com.patviafore.lox.Expr.Binary;
import com.patviafore.lox.Expr.Grouping;
import com.patviafore.lox.Expr.Literal;
import com.patviafore.lox.Expr.Ternary;
import com.patviafore.lox.Expr.Unary;
import com.patviafore.lox.Stmt.Expression;
import com.patviafore.lox.Stmt.Print;

public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {

    private Environment environment = new Environment();

    public void interpret(List<Stmt> statements) {
        try {
            for(Stmt statement: statements) {
                execute(statement);
            }
        }
        catch(RuntimeError error) {
            Lox.runtimeError(error);
        }
    }

    private void execute(Stmt stmt) {
        stmt.accept(this);
    }

    public String stringify(Object object) {
        if(object == null) return "nil";
        if(object instanceof Double) {
            String text = object.toString();
            if(text.endsWith(".0")) {
                text = text.substring(0, text.length() - 2);
            }
            return text;
        }
        return object.toString();
    }

    @Override
    public Object visitTernaryExpr(Ternary expr) {
        switch(expr.operator.type){
            case QUESTION: return isTruthy(evaluate(expr.condition) )? evaluate(expr.left) : evaluate(expr.right);
            default: return null;
        }
    }

    @Override
    public Object visitBinaryExpr(Binary expr) {
        Object left = evaluate(expr.left);
        Object right = evaluate(expr.right);

        switch(expr.operator.type){
            case GREATER: 
                checkNumberOperands(expr.operator, left, right);
                return(double)left > (double)right;
            case GREATER_EQUAL: 
                checkNumberOperands(expr.operator, left, right);
                return(double)left >= (double)right;
            case LESS: 
                checkNumberOperands(expr.operator, left, right);
                return(double)left < (double)right;
            case LESS_EQUAL: 
                checkNumberOperands(expr.operator, left, right);
                return(double)left < (double)right;
            case BANG_EQUAL: return !isEqual(left,right);
            case EQUAL_EQUAL: return isEqual(left,right);
            case MINUS: 
                checkNumberOperands(expr.operator, left, right);
                return (double)left - (double)right;
            case SLASH: 
                checkNumberOperands(expr.operator, left, right);
                if((double) right == 0) {
                    throw new RuntimeError(expr.operator, "Division by zero detected");
                }
                return (double)left / (double)right;
            case STAR: 
                checkNumberOperands(expr.operator, left, right);
                return (double)left*(double)right;
            case PLUS:
                if(left instanceof Double && right instanceof Double){
                    return (double)left + (double)right;
                }

                if(left instanceof String){
                    return (String)left + stringify(right);
                }
                if(right instanceof String) {
                    return stringify(left) + (String) right;
                }

                throw new RuntimeError(expr.operator, "Operands must be two numbers or two strings");

            default:    break;
        }

        return null;
    }

    private void checkNumberOperands(Token operator, Object left, Object right) {
        if(left instanceof Double && right instanceof Double) return;
        throw new RuntimeError(operator, "Operands must be numbers");
    }

    private boolean isEqual(Object a, Object b) {
        if(a == null && b == null) return true;
        if(a == null) return false;
        return a.equals(b);
    }

    @Override
    public Object visitGroupingExpr(Grouping expr) {
        return evaluate(expr.expression);
    }

    public Object evaluate(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public Object visitLiteralExpr(Literal expr) {
        return expr.value;
    }

    @Override
    public Object visitUnaryExpr(Unary expr) {
        Object right = evaluate(expr.right);

        switch(expr.operator.type) {
            case MINUS:
                checkNumberOperand(expr.operator, right);
                return -(double)right;
            case BANG:
                return !isTruthy(right);
            default:
                return null; // shouldn't reach here
        }
    }

    private void checkNumberOperand(Token operator, Object operand) {
        if (operand instanceof Double) return;
        throw new RuntimeError(operator, "Operand must be a number");
    }

    private boolean isTruthy(Object object) {
        if(object == null) return false;
        if(object instanceof Boolean) return (boolean) object;
        return true;
    }

    @Override
    public Void visitExpressionStmt(Expression stmt) {
        evaluate(stmt.expression);
        return null;
    }

    @Override
    public Void visitPrintStmt(Print stmt) {
        Object value = evaluate(stmt.expression);
        System.out.println(stringify(value));
        return null;
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt){
        Object value = null;
        if(stmt.initializer instanceof Uninitialized) {
            environment.defineUnitialized(stmt.name.lexeme);
            return null;
        }
        if(stmt.initializer != null) {
            value = evaluate(stmt.initializer);
        }
        environment.define(stmt.name.lexeme, value);
        return null;
    }

    @Override
    public Object visitVariableExpr(Expr.Variable expr){
        return environment.get(expr.name);
    }

    @Override
    public Object visitAssignExpr(Expr.Assign expr){
        Object value = evaluate(expr.value);
        environment.assign(expr.name, value);
        return value;
    }
    
    @Override
    public Void visitBlockStmt(Stmt.Block stmt){
        executeBlock(stmt.statements, new Environment(environment));
        return null;
    }

    void executeBlock(List<Stmt> statements, Environment environment) {
        Environment previous = this.environment;
        try {
            this.environment = environment;

            for (Stmt statement : statements) {
                execute(statement);
            }
        }
        finally {
            this.environment = previous;
        }
    }
}
