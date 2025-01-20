package com.patviafore.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.stream.Collectors;

public class LoxList extends LoxInstance {

    private static class LoxListClass extends LoxClass {
        LoxListClass(Environment environment) {
            super("list", null, new ArrayList<LoxClass>(), new HashMap<String, LoxFunction>());
            this.methods.put("size", new LoxNativeMethod("size", new ArrayList<Token>(), environment, new LoxCallable() {
                @Override
                public int arity() { 
                    return 0;
                }

                @Override
                public Object call(Interpreter interpreter, List<Object> arguments) {
                    return ((LoxList)arguments.get(0)).size();
                }
            }));
            Token value = new Token(TokenType.IDENTIFIER, "value", "value", 0);
            ArrayList<Token> appendParams = new ArrayList<Token>();
            appendParams.add(value);
            this.methods.put("append", new LoxNativeMethod("append", appendParams, environment, new LoxCallable() { 
                @Override
                public int arity() {
                    return 1;
                }

                @Override
                public Object call(Interpreter interpreter, List<Object> arguments) {
                    ((LoxList)arguments.get(0)).add(arguments.get(1));
                    return null;
                }
            }));

            this.methods.put("eraseAt", new LoxNativeMethod("eraseAt", appendParams, environment, new LoxCallable() { 
                @Override
                public int arity() {
                    return 1;
                }

                @Override
                public Object call(Interpreter interpreter, List<Object> arguments) {
                    ((LoxList)arguments.get(0)).eraseAt(arguments.get(1));
                    return null;
                }

            }));


        }
    }


    public LoxList(ArrayList<Object> contents, Environment environment) {
        super(new LoxListClass(environment));
        this.environment = environment;
        this.contents = contents;
    }
    
    public int size() {
        return contents.size();
    }

    public Object at(Token t, Object indexObject){
        try {
            int index = ((Double)indexObject).intValue();
        
            if(index < 0 || index >= contents.size()){
                throw new RuntimeError(t, "Index out of bounds on list");
            }
            
            return this.contents.get(index);
        }
        catch(ClassCastException e){
            throw new RuntimeError(t, "Index must be an integer");
        }
    }

    public void set(Token t, Object indexObject, Object value) {
        try {
            int index = ((Double)indexObject).intValue();
        
            if(index < 0 || index >= contents.size()){
                throw new RuntimeError(t, "Index out of bounds on list");
            }
            
            this.contents.set(index, value);
        }
        catch(ClassCastException e){
            throw new RuntimeError(t, "Index must be an integer");
        }
    }

    public void add(Object value) {
        this.contents.add(value);
    }
    
    public void eraseAt(Object indexObject) {
        try {
            int index = ((Double)indexObject).intValue();
        
            if(index < 0 || index >= contents.size()){
                throw new RuntimeException("Index out of bounds on list");
            }
            
            this.contents.remove(index);
        }
        catch(ClassCastException e){
            throw new RuntimeException("Index must be an integer");
        }
    }

    public static LoxList add(LoxList lhs, LoxList rhs) {
        ArrayList<Object> contents = new ArrayList<Object>();
        contents.addAll(lhs.contents);
        contents.addAll(rhs.contents);
        return new LoxList(contents, lhs.environment);

    }

    @Override
    public String toString() { 
        return "[" + this.contents.stream().map((Object o) -> StringUtils.stringify(o, true) ).collect(Collectors.joining(", ")) + "]";
    }

    private ArrayList<Object> contents;
    private Environment environment;
}
