package com.patviafore.lox;

public class StringUtils {
    
    public static String stringify(Object object){
        return stringify(object, false);
    }
    public static String stringify(Object object, boolean quoteStrings) {
        if(object == null) return "nil";
        if(object instanceof Double) {
            String text = object.toString();
            if(text.endsWith(".0")) {
                text = text.substring(0, text.length() - 2);
            }
            return text;
        }
        if(quoteStrings && object instanceof String){
            return "\"" + object.toString() + "\"";
        }
        return object.toString();
    }
}
