package com.patviafore.lox;

import java.util.Map;
import java.util.HashMap;

public class Environment {
    final Environment enclosing;
    private final Map<String, Object> values = new HashMap<>();

    Environment() {
        enclosing = null;
    }

    Environment(Environment enclosing) {
        this.enclosing = enclosing;
    }

    void define(String name, Object value){
        values.put(name, value);
    }

    void defineUnitialized(String name) {
        values.put(name, new Uninitialized());
    }

    void assign(Token name, Object value){
        if(values.containsKey(name.lexeme)) {
            values.put(name.lexeme, value);
            return;
        }

        if(enclosing != null) {
            enclosing.assign(name, value);
            return;
        }
        
        throw undefinedVariable(name);
    }

    Object get(Token name) {
        if(values.containsKey(name.lexeme)){
            Object obj = values.get(name.lexeme);
            if(obj instanceof Uninitialized) throw new RuntimeError(name, "Uninitialized variable: " + name.lexeme + ".");
            return obj;
        }

        if(enclosing != null) return enclosing.get(name);

        throw undefinedVariable(name);

    }

    RuntimeError undefinedVariable(Token name) {
        return new RuntimeError(name, "Undefined variable: " + name.lexeme + "." );
    }
    
}
