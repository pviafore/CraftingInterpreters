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

    `BLOCKCOMMENT = (<BLOCKCOMMENT> | .)*`

    See Scanner.java for information on how to do this.
