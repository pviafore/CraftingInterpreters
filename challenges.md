# Chapter 1

1)  What are at least six DSLs used in the system used to write this book.

    Makefiles, Markdown, YAML, HTML, HTAccess(INI), CSS

2)  Write a hello world in java

    See challenges/helloworld.java

3)  Write a hello world in C, and a doubly linked list

    See challenges/helloworld.c

# Chapter 2

1)  Pick a language and see how it does parsing/lexing

    Python actually has it's own hand-rolled lexer and parser: https://github.com/python/cpython/tree/main/Parser

2)  What are some reasons not to use JIT?

    If you need very deterministic performance, JIT may not be what you want, since the performance of when things are JIT'd or not may interfere with that.

    In code that is self modifying, you may not want to JIT either, but that would be it's own can of worms.

3)  Most LISP interpreters compile to C, but also let you interpret LISP on the fly. Why?

    Becasue of REPL. The feedback loop inherent in a REPL makes it really easy to prototype and try out ideas, without having to do an entire compile.


# Chapter 3

1)  Play around with some programs with some edge cases, does it do what you want?

    I did a recursive closure and that worked great, although it only referenced variables defined before it. 

    I tried nesting classes, but that blew up.

2)  What open questions do you have?

    - Are there value or reference semantics when passing parameters?
    - Are copies shallow or deep? 
    - Where are the collection types?
    - When are types destructed?
    - How is unicode handled?
    - Is there any sort of version semantics and does there exist backwards compatibility capabilities?


3) What are some features that you feel are missing in Lox

    Ignoring standard library, I feel like access specification is missing in classes for encapsulation purposes, I also haven't seen an else if, but might literally be just an else followed by an if ( I don't know if you support single line else and if). A switch statement might be nice too. Lastly, data collections. How do I do an array or an associative map?
  
# Chapter 4

1)  Why is Python and Haskell not considered regular

    In order to show they are not regular, you need to demonstrate that they have context-sensitive situations.

    For Python, imagine the following code

    ```
    try:
        pass
    except:
        pass
    ```

    You might think that the grammar is something like this:

    ```
    <TRY> <COLON>
    <INDENT> <BLOCK>
    <EXCEPT> <COLON>
    <INDENT><BLOCK>
    ```

    But the whole block has to be indented, when you are in a try statement, but there's no way to tell that in the grammar (because some blocks are not indented).

    Furthermore, what if this try-except block is itself in indentation. Then the try block needs indentation as well. That is, the grammar needs the context of indentation level to appropriately write itself.


    And as far as Haskell goes, it has similar indentation rules to Python, so it runs into the same problem.

2) Whitespace is not ignored in Ruby, Coffeescript and C, What are the effects of not ignoring whitespace in those languages.

    With Ruby, multiline comments are an issue, as the =begin and =end have to be at the beginning of the line.

    CoffeeScript uses white space for indentations and you have to have them in certain places

    The C preprocessor will do different things based on a macro foo() and foo ().

3)  Why might you want to write a scanner to not discard comments and whitespace?

    Scanners might not just be used with compilation/interpretaion. Many tools might want to parse this grammar. Think doxygen, automatic documentation generators, AI analysis tools, etc. that want to know what comments say.

    Additionally, you might run into things where you want to measure code complexity, and levels of indentation is a fantastic proxy for that.

4)  Add support for block-style comments.

    So we want these to be able to nest, so if we find an OPENCOMMENT token, we will keep reading until we find a CLOSECOMMENT token. We are going to ignore // tokens for the time being and treat block comments as the source of truth. Now for nesting, we need to have a counter of depth of comments, so that we can do something like this /* /* */ */ and not just exit at the first one (only exit the closecomment when our depth is zero). 

    From a grammar perspective, you'd have something like

    `BLOCKCOMMENT = /* (<BLOCKCOMMENT> | .)* */`

    See Scanner.java for information on how to do this.

# Chapter 5

1)  Rewrite the grammar without notational sugar:

    ```
    expr -> ( "(" ( expr (", expr)* )? ")" | "." IDENTIFIER )+
        |   NUMBER
        |   IDENTIFIER
    ```

    It can be simplified as follows:

        expr -> identifier
        expr -> number
        expr -> expr subexpr

        subexpr -> subexpr subexpr
        subexpr -> "." IDENTIFIER
        subexpr -> "(" args ")"

        args -> expr
        args -> expr "," args

    This gives you grammars like abc.def(ghi, 5.3, jkl). You now have nested function calls. You can get some silly things like 1.2.3(4.5.6), which doesn't quite work, but we can catch that in the parser.

2)  If a Visitor pattern lets us add functions really easily on types, what is the equivalent in a functional language to add new types that has a bundle of operations.

    You can use the factory pattern. Using Elixir:

    `def create_obj() do %{ :foo => doSomething, :bar => doSomethingElse}  end`

    THis way, you can create a map that has the foo and bar functions automatically on it(you can override them by setting them) You can now pass this map into anything that might need a foo or bar. (you could also make a method that takes a map and adds these to it)

3)  Make a Reverse Polish Notation Printer 

    See [RpnPrinter.java](jlox/jlox/src/main/java/com/patviafore/lox/RpnPrinter.java)

# Chapter 6

1)  Add a comma expression.

    See the codebase.

    Grammar is as follows:

        comma -> equality (, equality)*

2)   Add support for ternary operator?.  What is it's precedence and is it left-associative or right-associative.  

    See the code for ternary operations. It is right associative, as it needs to make sure that it you don't get a weird condition where both conditions are true in a nested, such as 

    `b==c ? true : true ? true : false`

    In this case, if you were left associative, you would have `b==c ? true : true` which will always be true, which might confuse people.

    As far as the precedence of the middle, it's assumed that it always is an entire expression, so it has pretty high precedence.

3)  Add error handling when a binary expression is missing a left operator.

    See code.

# Chapter 7

1)  Would you extend Lox to support comparing other types? If so, which pairs of types do you allow and how do you define their ordering? Justify your choices and compare them to other languages.

    I would allow user-defined comparisons ala C++ or Python, where you define a comparator on an object. Maybe Strings could have it so that it has default lexicographical ordering, but it should be an opt-in thing. By default, there shouldn't be mixed comparisons, because I think that strays too far into implicit casting, which nears JavaScript territory, which I'm not a fan of.

    I prefer C++'s `operator<` compared to Python's `__lt__` because I think it converys the usage better. 

2)  Make string concatenation behave if either element is a String.

    See Interpreter.java.

3)  What happens if you divide by zero? What do you think should happen? What do other languages do?

    Right now, it returns infinity. I've seen a variety of different rationale:

*   C++ has UB (YOLO!). Floats do IEEE, which is actually well defined. I assume this is because you don't need to do a whole lot of error checking on division, which speeds things up.
*   Java and Python have exceptions when you divide by zero, allowing users to check for it and catch it.
*   Pony, sets it to zero, which you can read about [here](https://tutorial.ponylang.io/gotchas/divide-by-zero.html)

    See Interpreter.java for handling this. I'm choosing to have a runtime error as I think a user deserves to catch it, but not crash the program. I am explicitly not handling IEEE floats, because I am not distinguishing between them in the code.

# Chapter 8

1)  Make it so that the REPL can take an expression or statement

    See [Lox.java](jlox/jlox/src/main/java/com/patviafore/lox/Lox.java). It now checks to see if there are any semicolons in the statement, and if there are, treats it as a statement, otherwise, it assumes a single expression.

2)  Make it so you have to initialize variables

    In [Environment.java](jlox/jlox/src/main/java/com/patviafore/lox/Environment.java), we are checking if the value is set to to a special type called Uninitialized (We want to have null values be okay to use)

3)  What happens if you shadow a variable and initialize it to the outer variable?

    It actually behaves as I expect, by evaluating the rhs first. In other languages, it doesn't quite behave this way. Python throws an unbound error, and C/C++ has UB.

    