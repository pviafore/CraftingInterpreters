package com.patviafore.lox;

import java.util.List;

public class LoxClosure implements LoxCallable {

    private final Expr.Closure declaration;
    private final Environment closure;

    LoxClosure(Expr.Closure declaration, Environment closure) { 
        this.declaration = declaration;
        this.closure = closure;
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
            return returnValue.value;
        }
        return null;
    }

    @Override
    public String toString() {
        return "<fun <anonymous> >";
    }
    
}
