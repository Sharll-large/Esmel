# Esmel
Welcome to the language of Esmel !

Now the Esmel has already had some basic features.

The detailed language booklet will be written soon.

For example, you can write an Esmel script like this:

```Esmel
fn fib x
    return 1 if equal? x 1
    return 1 if equal? x 2
    return add fib sub x 1 fib sub x 2

fn hi sth
    print "Hello, "
    print sth
    println "!"

fn main
    set "me" "Esmel"
    hi me

    print "Fib(10): "
    println fib 10

    println "Counting from 1-10:"
    set "i" 0
    flag Start
      set "i" add i 1
      println i
    goto "Start" if ! equal? i 10
```

As you see, Esmel already supports many features required by programming languages, such as functions, local variables, recursion, branching, loops.

