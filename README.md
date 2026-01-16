# Esmel
### Welcome to the language of Esmel !
##### Current Version: 3.7pre4

---

#### Now Esmel has already supported many basic features.(Function, Array, GC, etc.)

#### Also, after my weeks of improving, Esmel has a very good performance.

#### The Esmel Code is completely open-sourced, well-organized and logical(maybe), so you can read the source code to learn more about programming languages.

---
#### A detailed language booklet will be written soon.

#### But for now, you can write a simple Esmel script like this:

```Shell 
Function Main
    # We designed a program to show the basic usages of Esmel
    # and test Esmel's basic math performance.

    # In esmel, we use `Set` to edit a variable.
    Set sum 0
    Set i 1
    # Set a timer to measure the time.
    Set startTime CurrentTime

    # Define a flag, so that we can repeat adding something.
    flag start
        Add sum i
        Add i 1
        end If Equal? i 100001
        start   # Means Goto the flag `start`.
    flag end

    Set timeSum - CurrentTime startTime

    Println "Sum of 1~100000: "
    Println sum
    Print   "Used time(ms): "
    Print   timeSum
    # On Sharll's computer, it takes 5ms, which is 100% faster than Python.
```

