package com.patviafore.lox;

import java.util.List;

public class LoxFunction implements LoxCallable {

    private final Stmt.Function declaration;
    protected final Environment closure;
    private final boolean isInitializer;

    LoxFunction(Stmt.Function declaration, Environment closure, boolean isInitializer) { 
        this.declaration = declaration;
        this.closure = closure;
        this.isInitializer = isInitializer;
    }
    @Override
    public int arity() {
        return declaration.params.size(); 
    }
    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        Environment environment = new Environment(closure);
        for(int i = 0; i < declaration.params.size(); i++){
            environment.define(declaration.params.get(i).lexeme, arguments.get(i));
        }
        try {
            interpreter.executeBlock(declaration.body, environment);
        }
        catch(Return returnValue) {
            if(isInitializer && !declaration.isStaticMethod) return closure.getAt(0, 0); // assume this is the first thing defined
            return returnValue.value;
        }
        return null;
    }

    List<Token> getParams() {
        return declaration.params;
    }

    @Override
    public String toString() {
        return "<fn " + declaration.name.lexeme + ">";
    }

    LoxFunction bind(LoxInstance instance) {
        Environment environment = new Environment(closure);
        environment.define("this", instance);
        return new LoxFunction(declaration, environment, isInitializer);
    }

    public boolean isStatic() {
        return declaration.isStaticMethod;
    }
    
    public boolean isProperty() {
        return declaration.isProperty;
    }
    
}
