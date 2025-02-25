package com.patviafore.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.patviafore.lox.Expr.Binary;
import com.patviafore.lox.Expr.Call;
import com.patviafore.lox.Expr.Closure;
import com.patviafore.lox.Expr.Grouping;
import com.patviafore.lox.Expr.Literal;
import com.patviafore.lox.Expr.Ternary;
import com.patviafore.lox.Expr.Unary;
import com.patviafore.lox.Stmt.Break;
import com.patviafore.lox.Stmt.Expression;
import com.patviafore.lox.Stmt.Function;
import com.patviafore.lox.Stmt.If;
import com.patviafore.lox.Stmt.Print;

public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {

    private class NodeMetadata {

        NodeMetadata(int distance, int offset) {
            this.distance = distance;
            this.offset = offset;
        }
        int distance = 0;
        int offset = 0;

    }
    final Environment globals = new Environment();
    private final Map<Expr, NodeMetadata> locals = new HashMap<>();
    private Environment environment = globals;
    private int loopDepth = 0;
    private Boolean hitBreak = false;

    Interpreter() {
        globals.define("clock", new LoxCallable(){ 
            @Override
            public int arity() { return 0; }

            @Override
            public Object call(Interpreter interpreter, List<Object> arguments) {
                return (double) System.currentTimeMillis() / 1000.0;
            }

            @Override
            public String toString() { return "<native fn>";}
        });
    }

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

    public void interpretExpression(Expr expression) {
        try {
            System.out.println(stringify(evaluate(expression)));
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
                return(double)left <= (double)right;
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
            case COMMA:
                return right;
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
        return lookUpVariable(expr.name, expr);
    }

    private Object lookUpVariable(Token name, Expr expr) {
        NodeMetadata metadata = locals.get(expr);
        if(metadata != null) {
            return environment.getAt(metadata.distance, metadata.offset);
        } else {
            return globals.get(name);
        }
    }

    @Override
    public Object visitAssignExpr(Expr.Assign expr){
        Object value = evaluate(expr.value);

        NodeMetadata metadata = locals.get(expr);
        if(metadata != null) {
            environment.assignAt(metadata.distance, metadata.offset, value);
        } else {
            globals.assign(expr.name, value);
        }
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
                if(hitBreak && loopDepth > 0){
                    break;
                }
                execute(statement);
            }
        }
        finally {
            this.environment = previous;
        }
    }

    @Override
    public Void visitIfStmt(If stmt) {
        if (isTruthy(evaluate(stmt.condition))){
            execute(stmt.thenBranch);
        }
        else if (stmt.elseBranch != null) {
            execute(stmt.elseBranch);
        }
        return null;
    }

    @Override
    public Object visitLogicalExpr(Expr.Logical expr) {
        Object left = evaluate(expr.left);

        if (expr.operator.type == TokenType.OR) {
            if(isTruthy(left)) return left;
        }
        else {
            if(!isTruthy(left)) return left;
        }

        return evaluate(expr.right);
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        try {
            loopDepth++;
            while (isTruthy(evaluate(stmt.condition))) {
                execute(stmt.body);
                if(hitBreak) {
                    hitBreak = false;
                    break;
                }
            }
            return null;
        }
        finally {
            loopDepth--;
        }
    }

    @Override
    public Void visitBreakStmt(Break stmt) {
        hitBreak = true;
        return null;
    }

    @Override
    public Object visitCallExpr(Call expr) {
        Object callee = evaluate(expr.callee);
        List<Object> arguments = new ArrayList<>();
        for (Expr argument : expr.arguments) {
            arguments.add(evaluate(argument));
        }

       if(!(callee instanceof LoxCallable)) {
            throw new RuntimeError(expr.paren, "Can only call functions and classes.");
        }

        LoxCallable function = (LoxCallable)callee;

        if(arguments.size() != function.arity()) {
            throw new RuntimeError(expr.paren, "Expected " + function.arity() + " arguments but got " + arguments.size() + ".");
        }
        return function.call(this, arguments);
    }

    @Override
    public Object visitClosureExpr(Closure expr) {
        LoxClosure f = new LoxClosure(expr, environment);
        return f;
    }

    @Override
    public Void visitFunctionStmt(Function stmt) {
        LoxFunction function = new LoxFunction(stmt, environment, false);
        environment.define(stmt.name.lexeme, function);
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        Object value = null;
        if (stmt.value != null) value = evaluate(stmt.value);

        throw new Return(value);
    }

    void resolve(Expr expr, int depth, int index) {
        locals.put(expr, new NodeMetadata(depth, index));
    }

    @Override
    public Void visitClassStmt(Stmt.Class stmt) {
        Object superclass = null;
        if(stmt.superclass != null) {
            superclass = evaluate(stmt.superclass);
            if(!(superclass instanceof LoxClass)) {
                throw new RuntimeError(stmt.superclass.name, "Superclass must be a class.");
            }

        }
        ArrayList<LoxClass> mixins = new ArrayList<LoxClass>();
        for(Expr.Variable mixin : stmt.mixins) {
            Object mixinclass = evaluate(mixin);
            if(!(mixinclass instanceof LoxClass)) {
                throw new RuntimeError(mixin.name, "Mix-in must be a class.");
            }
            mixins.add((LoxClass)mixinclass);
        }
        environment.define(stmt.name.lexeme, null);

        if (superclass != null) {
            environment = new Environment(environment);
            environment.define("super", superclass);
        }

        Map<String, LoxFunction> methods = new HashMap<>();
        for(Stmt.Function method: stmt.methods){
            LoxFunction function = new LoxFunction(method, environment, method.name.lexeme.equals("init"));
            methods.put(method.name.lexeme, function);
        }

        LoxClass cls = new LoxClass(stmt.name.lexeme, (LoxClass)superclass, mixins, methods);

        if(superclass != null) {
            environment = environment.enclosing;
        }
        environment.assign(stmt.name, cls);
        return null;
    }

    @Override
    public Object visitGetExpr(Expr.Get expr){
        Object object = evaluate(expr.object);
        if(object instanceof LoxInstance) {
            Object subobject = ((LoxInstance) object).get(expr.name);
            if(subobject instanceof LoxCallable && ((LoxFunction)subobject).isProperty()){
                //immediately call property
                return ((LoxFunction)subobject).call(this, new ArrayList<Object>());
            }

            return subobject;
        }
        else if(object instanceof LoxClass) {
            return ((LoxClass) object).getStatic(expr.name);
        }

        throw new RuntimeError(expr.name, "Only instances have properties");
    }
    
    @Override
    public Object visitSetExpr(Expr.Set expr){
        Object object = evaluate(expr.object);
        if(object instanceof LoxInstance) { 
            Object value = evaluate(expr.value);
            ((LoxInstance) object).set(expr.name, value);
            return value;
        }
        
        throw new RuntimeError(expr.name, "Only instances have properties");
    }

    @Override
    public Object visitThisExpr(Expr.This expr) {
        return lookUpVariable(expr.keyword, expr);
    }

    @Override
    public Object visitSuperExpr(Expr.Super expr) {
        NodeMetadata metadata = locals.get(expr);
        LoxClass superclass = (LoxClass) environment.getAt(metadata.distance, metadata.offset);
        LoxInstance object = (LoxInstance)environment.getAt(metadata.distance - 1, 0); // assume this is always at offset 0 in the above scope
        LoxFunction method = superclass.findMethod(expr.method.lexeme, expr.method);

        if(method == null){
            throw new RuntimeError(expr.method, "Undefined property '" + expr.method.lexeme + "'.");
        }
        return method.bind(object);
    }

}
