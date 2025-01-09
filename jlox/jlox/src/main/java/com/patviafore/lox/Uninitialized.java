package com.patviafore.lox;

public class Uninitialized extends Expr {

    @Override
    <R> R accept(Visitor<R> visitor) {
        throw new UnsupportedOperationException("Unimplemented method 'accept'");
    }
    
}
