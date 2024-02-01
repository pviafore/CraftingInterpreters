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
  
