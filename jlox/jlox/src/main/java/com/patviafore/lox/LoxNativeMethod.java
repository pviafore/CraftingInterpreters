package com.patviafore.lox;
import java.util.ArrayList;
import java.util.List;
public class LoxNativeMethod extends LoxFunction {

    public LoxNativeMethod(String name, List<Token> params, Environment environment, LoxCallable callable) {
        super(makeFunction(name, params), environment, false);

        this.name = name;
        this.params = params;
        this.callable = callable;
    }

    private static Stmt.Function makeFunction(String name, List<Token> params) {
        String tokenName = "<built-in " + name + ">";
        Token builtin = new Token(TokenType.IDENTIFIER, tokenName, tokenName, 0);
        return new Stmt.Function(builtin, params, new ArrayList<Stmt>(), false, false);
    }

    @Override
    public int arity() {
        return callable.arity();
    }
    
    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        arguments.add(0, closure.getAt(0, 0));
        Object obj = callable.call(interpreter, arguments);
        return obj;
    }

    @Override
    LoxFunction bind(LoxInstance instance) {
        Environment environment = new Environment(closure);
        environment.define("this", instance);
        return new LoxNativeMethod(name, params, environment, callable);
    }

    private LoxCallable callable;
    private String name;
    private List<Token> params;
}
