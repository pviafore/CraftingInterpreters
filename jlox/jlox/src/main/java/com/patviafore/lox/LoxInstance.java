package com.patviafore.lox;

import java.util.HashMap;
import java.util.Map;

public class LoxInstance {
    protected LoxClass cls;
    private final Map<String, Object> fields = new HashMap<>();

    LoxInstance(LoxClass cls){ 
        this.cls = cls;
    }

    @Override
    public String toString() {
        return cls.name + " instance";
    }

    Object get(Token name, boolean walkSuperChain) { 

        LoxFunction method = cls.findMethod(name.lexeme, name, walkSuperChain);
        if(method != null) return method.bind(this);
        
        if(fields.containsKey(name.lexeme)){
            return fields.get(name.lexeme);
        }

        throw new RuntimeError(name, "Undefined property '" + name.lexeme + "'");
    }

    void set(Token name, Object value) {
        fields.put(name.lexeme, value);
    }

    LoxClass getClassType(){
        return cls;
    }

}
