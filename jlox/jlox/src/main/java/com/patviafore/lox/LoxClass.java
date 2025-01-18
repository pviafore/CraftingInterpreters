package com.patviafore.lox;

import java.util.List;
import java.util.Map;

public class LoxClass implements LoxCallable {
    final String name;
    final LoxClass superclass;
    private final Map<String, LoxFunction> methods;

    LoxClass(String name, LoxClass superclass, Map<String, LoxFunction> methods) {
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
    }

    @Override
    public String toString() { 
        return name;
    }

    public int arity() {
        LoxFunction initializer = findMethod("init");
        if(initializer == null) return 0;

        return initializer.arity();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        LoxInstance instance = new LoxInstance(this);
        LoxFunction initializer = findMethod("init");
        if(initializer != null) {
            initializer.bind(instance).call(interpreter, arguments);
        }
        return instance;
    }

    public Object getStatic(Token name) {
        LoxFunction staticMethod = findMethod(name.lexeme);
        if(!staticMethod.isStatic()) throw new RuntimeError(name, "Only static methods are allowed after a class name");
        return staticMethod;
    }

    public LoxFunction findMethod(String name) {
        if(methods.containsKey(name)){
            return methods.get(name);
        }

        if(superclass != null){ 
            return superclass.findMethod(name);
        }

        return null;
    }
}
