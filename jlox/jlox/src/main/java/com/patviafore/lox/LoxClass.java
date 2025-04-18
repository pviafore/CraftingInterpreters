package com.patviafore.lox;

import java.util.List;
import java.util.Map;

public class LoxClass implements LoxCallable {
    final String name;
    final LoxClass superclass;
    private final List<LoxClass> mixins;
    private final Map<String, LoxFunction> methods;

    LoxClass(String name, LoxClass superclass, List<LoxClass> mixins, Map<String, LoxFunction> methods) {
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
        this.mixins = mixins;
    }

    @Override
    public String toString() { 
        return name;
    }

    public int arity() {
        LoxFunction initializer = findMethod("init", null);
        if(initializer == null) return 0;

        return initializer.arity();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        LoxInstance instance = new LoxInstance(this);
        LoxFunction initializer = findMethod("init", null);
        if(initializer != null) {
            initializer.bind(instance).call(interpreter, arguments);
        }
        return instance;
    }

    public Object getStatic(Token name) {
        LoxFunction staticMethod = findMethod(name.lexeme, name);
        if(!staticMethod.isStatic()) throw new RuntimeError(name, "Only static methods are allowed after a class name");
        return staticMethod;
    }

    public LoxFunction findMethod(String name, Token tok) {

        if(methods.containsKey(name)){
            return methods.get(name);
        }

        LoxFunction f = null;

        if(superclass != null){ 
            f = superclass.findMethod(name, tok);
        }
        
        if(name != "init"){

            for(LoxClass mixin: mixins ) {
                LoxFunction mixin_f = mixin.findMethod(name, tok);
                if(mixin_f != null && f != null) {
                    throw new RuntimeError(tok, "Ambiguous method found in mixins");
                }
                if(mixin_f != null){
                    f = mixin_f;
                }
            }
        }

        return f;
    }
}
