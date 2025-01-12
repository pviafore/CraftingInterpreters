package com.patviafore.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

public class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void>{
    private final Interpreter interpreter;
    private class NodeMetadata { 
        NodeMetadata(boolean isDefined, Token token, int index) { 
            this.isDefined = isDefined;
            this.token = token;
            this.index = index;
        }

        public boolean isDefined = false;
        public boolean isReferenced = false;
        public boolean isFunction = false;
        public int index = 0;
        Token token;

    }

    private class Scope { 
        Scope() { 
            nodes = new HashMap<String,NodeMetadata>();
        }

        void checkForErrors() {
            nodes.forEach((name, metadata) -> {
                if(!metadata.isReferenced && metadata.isFunction == false){
                    Lox.error(metadata.token, "Unused local variable: " + name);
                }
            });
        }

        public boolean containsName(String name){
            return nodes.containsKey(name);
        }

        public void declare(Token name) {
            nodes.put(name.lexeme, new NodeMetadata(false, name, nodes.size()));
        }

        public void define(Token name) {
            nodes.get(name.lexeme).isDefined = true;
        }
        
        public void reference(Token name) {
            nodes.get(name.lexeme).isReferenced = true;
        }
        
        public void markAsFunction(Token name) {
            nodes.get(name.lexeme).isFunction = true;
        }

        public boolean isDeclaredButNotDefined(Token name) {
            return containsName(name.lexeme) && nodes.get(name.lexeme).isDefined == Boolean.FALSE; 
        }

        public int getIndex(Token name) {
            return nodes.get(name.lexeme).index;
        }

        private Map<String, NodeMetadata> nodes;
        private ArrayList<String> enclosedVariables = new ArrayList<String>();
    }
    private final Stack<Scope> scopes = new Stack<>();
    private FunctionType currentFunction = FunctionType.NONE;

    Resolver(Interpreter interpreter){
        this.interpreter = interpreter;
    }

    private enum FunctionType {
        NONE,
        FUNCTION,
        CLOSURE
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        beginScope();
        resolve(stmt.statements);
        endScope();
        return null;
    }

    void resolve(List<Stmt> statements) {
        for(Stmt statement: statements){
            resolve(statement);
        }
        checkForErrors();
    }

    private void checkForErrors() {
        for(Scope scope : scopes){
            scope.checkForErrors();
        }
    }

    void resolve(Stmt stmt) {
        stmt.accept(this);
    }

    private void resolve(Expr expr) {
        expr.accept(this);
    }

    private void beginScope() {
        scopes.push(new Scope());
    }

    private void endScope() {
        scopes.pop();
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt){
        declare(stmt.name);
        if(stmt.initializer != null) {
            resolve(stmt.initializer);
        }
        define(stmt.name);
        return null;
    }

    private void declare(Token name) {
        if(scopes.empty()) return;

        Scope scope = scopes.peek();
        if(scope.containsName(name.lexeme)) {
            Lox.error(name, "Already a variable with this name in this scope.");
        }
        scope.declare(name);
    }

    private void define(Token name) {
        if(scopes.isEmpty()) return;
        scopes.peek().define(name);
    }

    @Override
    public Void visitVariableExpr(Expr.Variable expr) {
        if(!scopes.isEmpty() && scopes.peek().isDeclaredButNotDefined(expr.name)){
            Lox.error(expr.name, "Can't read local variable in its own initializer");
        }

        resolveLocal(expr, expr.name);
        return null;
    }

    private void resolveLocal(Expr expr, Token name) {
        for(int i = scopes.size() - 1; i>= 0; i--){
            if(scopes.get(i).containsName(name.lexeme)){ 
                scopes.get(i).reference(name);
                interpreter.resolve(expr, scopes.size() - 1 - i, scopes.get(i).getIndex(name));
                return;
            }
        }
    }

    @Override
    public Void visitAssignExpr(Expr.Assign expr){
        resolve(expr.value);
        resolveLocal(expr, expr.name);
        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt){
        declare(stmt.name);
        define(stmt.name);
        if(!scopes.isEmpty()) {
            scopes.peek().markAsFunction(stmt.name); // for local functions - they can stay "unused"
        }
        resolveFunction(stmt, FunctionType.FUNCTION);
        return null;
    }

    public void resolveFunction(Stmt.Function function, FunctionType functionType) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = functionType;
        beginScope();
        for(Token param: function.params) { 
            declare(param);
            define(param);
        }
        resolve(function.body);
        endScope();
        currentFunction = enclosingFunction;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) { 
        resolve(stmt.expression);
        return null;
    }

    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        resolve(stmt.condition);
        resolve(stmt.thenBranch);
        if(stmt.elseBranch != null) resolve(stmt.elseBranch);
        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        resolve(stmt.expression);
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt){
        if(currentFunction == FunctionType.NONE) {
            Lox.error(stmt.keyword, "Can't return from the top-level code.");
        }
        if(stmt.value != null) {
            resolve(stmt.value);
        }
        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        resolve(stmt.condition);
        resolve(stmt.body);
        return null;
    }

    @Override
    public Void visitBinaryExpr(Expr.Binary expr) {
        resolve(expr.left);
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitCallExpr(Expr.Call expr) {
        resolve(expr.callee);
        for(Expr argument: expr.arguments) {
            resolve(argument);
        }
        return null;
    }

    @Override
    public Void visitGroupingExpr(Expr.Grouping expr) {
        resolve(expr.expression);
        return null;
    }

    @Override
    public Void visitLiteralExpr(Expr.Literal expr) {
        return null;
    }

    @Override
    public Void visitLogicalExpr(Expr.Logical expr) {
        resolve(expr.left);
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitUnaryExpr(Expr.Unary expr) {
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitBreakStmt(Stmt.Break stmt){
        return null;
    }

    @Override
    public Void visitClosureExpr(Expr.Closure function) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = FunctionType.CLOSURE;
        beginScope();
        for(Token param: function.params) { 
            declare(param);
            define(param);
        }
        resolve(function.body);
        endScope();
        currentFunction = enclosingFunction;
        return null;
    }

    @Override
    public Void visitTernaryExpr(Expr.Ternary expr) {
        resolve(expr.condition);
        resolve(expr.left);
        resolve(expr.right);
        return null;
    }
}
