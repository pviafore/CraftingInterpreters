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

    This gives you grammars like abc.def(ghi, 5.3, jkl). You now have nested function calls. You can get some silly things like 1(4.fed), which doesn't quite work, but we can catch that in the parser.

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

    NB: Looking at this at a later date, I don't see it in the code, so I assume it was removed with a refactor. You would have to check if the first token is a binary operator, (and not an unary operator), and if it is, parse the following expression, but throw an error and discard it.

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

# Chapter 9

1) How do languages with dynamic dispatch handle control flow with no if statement.

    If we take a look at Elixir, we can see how it does it. In essence, you define two functions with different tages

```
    iex(4)> foo = fn true -> 5    
    ...(4)>          false -> 4
    ...(4)> end
    #Function<42.3316493/1 in :erl_eval.expr/6>
    iex(5)> foo.(true)         
    5
    iex(6)> foo.(false)        
    4
```

    At runtime, the invocation will pick which one to choose.

2) How do languages handle looping with no control flow?

    Take a look at Haskell, which uses recursion to handle it's looping. However, it must have tail-call recursion, which lets the compiler replace a recursive function call with a jump to the beginning of the frame. Otherwise, a long loop would exhaust the stack.

3)  Add a break statement, allowing it to be nested in a loop.

# Chapter 10

1) What does smalltalk do for argument/parameter number mismatch?

    The arguments are actually part of the function name, so you're still just looking for a function name that matches those arguments, and no checking of how many commas you have or colons is needed.

2) Add anonymous function expressions

See the code for LoxClosure, and the Closure expression.

I had to tweak how I was parsing/interpreting expressions and statements to get fun(){}; to just work. We also now check to see if there is an anonymous function
when we get to parsing function statements, and if it is, we need to just use expression statement instead. This allows things like `fun(){ print "hello";}();`

3) What happens if you define a variable that shadows a parameter in a function body? What about in other languages.

Lox continues on merrily, as does Python and C++. JavaScript errors out on this when using `const` or `let`. 

 # Chapter 11

 1) Why is it safe to eagerly define the variable bound to a function's name as opposed to other variables

A function's name cannot be redefined in the same execution, the body happens in a separate scope. There is no risk of referring to the function when we bind the name or
reading a variable from the wrong scope.

2)  What happens when you try to define a local variable that refers to the same name.

Python will error out `UnboundLocalError: cannot access local variable 'x' where it is not associated with a value`, Putting `global` or using it in global space is fine though.

JavaScript does an error too -> can't access lexical declaration before initialization, which is the same as globals.

C++, unsurprisingly, let's it ride.

I like having it as an error explicitly, because I can't think of a reason why you would want to do this, as well as catching potential shadowing problems. If you needed
to do something for some reason, it's just an extra temp declaration or rename, which should be cheap.

3) Extend the resolver to report if a local variable is never used.

See the code in Resolver.java.

4) Handle environments by index based instead of map-based

See the code in Resolver.java, Environment.java, and Interpreter.java. 


# Chapter 12

1) Create static methods

Parser.java will read any class function as a static method. It will store in the AST that the method is static,
which the resolver and interpreter can use to know if they need to be calling a method on the class itself, or on the
instance. We can also verify that `this` can't be used in a static method.

2) Create properties on a class

Similar to static methods, I created a boolean that is passed through parse, resolve, and interpret to indicate if a function is a property.
When performing a get expression, if we are getting a property, immediately call the function and return the return value of that function
instead.

3)  What are the trade-offs over controlling encapsulation (forcing getters/setters and free access)

To me, it is a trade-off about developer experience vs developer safety. For languages that force encapsulation (or provide private/public
access), you are giving the developer tools to avoid mistakes earlier on. If a developer is messing with class internals, or if they are violating
invariants, you can avoid that completely by making those internals private. 

However, forcing users to do this may be overkill for smaller programs, so languages with a more dynamic flair may opt to eschew this safety entirely,
trusting developers to know how they want to use their objects best. Language implementers don't have to be concerned with setting up access rules,
and developers are free to change things more easily. Additionally, these specifiers or access control might be statically determined, which would need
additional semantic rules for how to handle dynamic fields being added to an object., ""

# Chapter 13

1) Many languages offer some form of multiple subclassing, where you have mix-ins, traits, multiple inheritance, etc. What is your preference and go implement it.

A long time ago, multiple inheritance would have been my pick, but I almost never use it, nor do I think it's that great of a design (Diamond of death, Python's convoluted MRO, etc.). Java only allows single inheritance, but you can inherit multiple interfaces. I have come to like this more, but interfaces are kinda a byproduct of static typing 
(I prefer generics with C++ more for this sort of thing). Really, I want one way that something can be a "is-a", but provide another way to reuse code. 
What I'm going to implement for my version of lox, I am going to hae single inheritance (that is, only one class can be the superclass), but allow multiple mix-ins. 

While `<` is used for subclassing, `+` will be used for mix-ins, with commas separating the clauses. Superclasses must be first.

 Any class can be used as a mix-in, but some things apply: 

* The constructor cannot be called for the mix-in, so state is discouraged
* You can have many mix-ins, but it will be a runtime error if you call a method that is in more than one mix-in (if we allowed Class.method, we might be able to, but that syntax isn't there yet -- but we do have a future way of resolving this from the caller)
* The mix-in's method are all available on the instantiated class
* Mix-ins may reference methods or variables that are defined on the instantiated class

This allows reuse and the basics of protocols, without promoting terrible inheritance chains.

Example: 

```
class Averagable { 
    average() {
        return this.sum() / this.count();
    }
}

class StudentScores +Avergable{ 
    sum() {
        return 500;
    }

    count() {
        return 6;
    }
}

print StudentScores().average();
```

2) Reverse the inheritance so that "inner" is used instead of super(), like in `Beta`

Check out the `beta-inner` branch.

3) I accidentally put this on beta-inner branch as well, whoops

# Chapter 14

1) Compress the line information for instructions

As suggested in the text, I am doing a RLE of a sort.

For line numbers, I store the line number, and the number of instructions it maps to. Then, I can iterate through
by each line (it's linear based on how many lines are in the source file). I can then check to see if the offset would
fall into the next lines block of instructions.  For example, if I had:

Line 1, 4 instructions

Line 2, 2 instructions

Line 3, 6 instructions

Let's say my offset is 7. I check if that would be less than 4 (instructions 0 -3), and if not, go on to the next line. The next line is
instructions 4-5, which 7 is not less than, but the next line is instrcuctions 6-11, which is a hit, so I know I'm in line 3.


2) Create a CONSTANT_LONG operation. What are some of the sacrifices

See code

We now have more opcodes, which I know some RISC enthusiasts may cringe at, especially for such a small edge case (after all, this is the sort of problem that 
we decry for Intel assembly architectures). We also have to increase the code for anything that is consuming the bytecode.

3) How does malloc and free work in regards to knowing the size, which bytes are allocated, and how do they deal with fragmentation. As 
   a hard mode, write your reallocate with an arena.

So I found this for malloc and free: https://sourceware.org/glibc/wiki/MallocInternals.

I've also been reading TAOCP from Knuth where the first volume talks a ton about pool allocators

So I think what I'm going to do is adopt Knuth's buddy system with a pool allocator. Essentially:

* Reserve 1 gig of memory into an arena.
* Set up linked lists for all sizes from 8 bytes to 1 gigabyte.
* At first, there is one block is 1 gigabyte in size
* Each block is a linked list with the first 4 being the next link in the list, next 4 being the previous link. The free block should have a 1 as the first bit.
* Upon allocation, we will walk up the sizes until we find something that can fit the memory (keeping in mind we need 4 bytes for bookkeeping)
* When we find a block that fits, we split it in half if needed, populating any below lists that fit the halves. The first bit will be come a zero for in use, and the next 31 bits is the power of 2 that is the size of the block (so we can have up to 2^127 with this scheme).
* We'll write the number of bits in the first byte, reserving the rest of the block for the actual memory. 
* When we free, we'll put the block on the front of the appropriate list, and look if it's corresponding other half is free (The first bit should be a 1). If it is, collapse it, put that block on its free store and repeat.


You could get fragmentation if you allocate everything, and then piecemeal deallocate small blocks, but there's not much help avoiding that, since we don't want to be 
moving blocks around. Since blocks are returned to the beginning of the list, the blocks that are freed will be the ones that were most recently used.

Note that I'm not being clever and handling larger sizes, this is just to show that we can do something with an arena. A true implementaiton would deal
with overcommitted memory, multiple arenas that are built on the fly, and so on.

# Chapter 15

1) Write teh bytecode instruction sequences for the follwoing:

1 + 2 + 3

* Constant 1
* Cosntant 2 
* Op Add
* Constant 3
* Op Add


1+2*3

* Constant 1
* Constant 2
* Constant 3
* Op Multiply
* Op add

3 - 2 - 1

* Constant 3
* Constant 2
* Op Subtract
* Constant 1
* Op Subtract

1 + 2 * 3 - 4 / - 5

* Constant 1
* Constant 2
* Constant 3
* Op Multiply
* Op Add
* Constant 4
* Constnat 5
* Negate
* Op Divide
* Op Subtract


2) Write the following without negate and without subtract. What are the trade-offs and are there other redundant instructions?


4 - 3 * -2

Without negate: 

* Constant 4
* Constnat 0
* Constant 3
* Constant 2
* Multiply
* Op Subtract
* Op Subtract


Wihtout subtraction

* Constant 4
* Constant 3
* Constant 2
* Op Negate
* Op Divide
* Op Negate
* Op Add


I like having both instructions, because while I think it does complicate for an implementer, it reduces the bytecode size, which will speed things up.

If it's something the user can directly do (like a negate, or a subtraction), it will also help with debugging the disassembly.


As far as redundancy, you don't need subtract if you have add, and you don't need multiply if you have a divide

3) Our VM stack has a fixed size, make it dynamic. What are the costs and benefits of it?

See the code to see a vector growing and shrinking the stack. This does add runtime cost, as we have to do memory management with a vector.
With a fixed stack, you have a few instrudtions to move the stack pointer, and no allocs needed. However, you can run into stack overflows if you
keep a static stack.


See the code where we use `DynamicStack`

4) Is there a big performance difference by eliding a pop when a push is going to come right after it? 

I'm not sure I'm going to measure enough of a difference, but it is going to be very small (it will be less instructions. I don't
feel like setting up benchmarking at this time to deal with this.) I'm altering the instructions to see their work.

# Chapter 16

1) What token types would you use for string interpolation and what would you emit for string interpolations?

So Python handles nested interpolations. At it's core, it feels like you have strings, that then terminate before you hit ${}. So for
something like "Hello ${name}", you would need to look for a $". You'd terminate the string at "Hello ", which is a string token, and then have
an interpolate token `${`. The closing brace would still be a `RIGHT_BRACE`. When you get to a parsing stage, you would evaluate all the tokens as an expression
between Interpolate and RightBrace. You would also need to start another string token after the right brace. You'd also want to implicitly converte the expression
to a string. You would also need implicit add operators between the string and interpolation and add operations between interpolation and next string.

2) How do langugaes differentiate between nested generics `vector<vector<int>>` and right shift `>>`. 

My guess is precedence checking and making things not context-free. Let's go check it out:

So in C++, they stopped looking at >> as a single token, and instead treated > as a single token. This way, when you were parsing template lists, 
you would be able to figure out which > delimites the token.

Java looks at context and if its in the context of a type, a > is always treated as a single character rather than being greedy as it is everywhere else.

C# does not treat `>>` as a separate token.

3) What are some contextual keywords from other languages? What are pros and cons of having contextual keywords. How would you implement them in the front-end.

Python has match, case, and type. Match and case are part of the new match operator.

C++ has coroutine contextual keywords (co_yield, co_return), and method specifiers (override, final).

This is nice because it gives you backwards compatibility - older programs will continue to compile. You won't run into scanning bugs because 
you aren't treating these any different, they are still just identifiers. However, it will make a language implementer's job much tougher. You now
have to do so much more for contextual awareness to know if you should be using an identifier or keyword. It also adds more contextual overhead for
developers because there are more rules to understand.

I would scan them as identifiers, and then in parse state, before any identifier that may be used in that context, you have to check if it's useful. (For example, 
if you were reading a function, you may look for identifiers in C++ after the parameter list, and see if they match override or final. )


# Chapter 17

