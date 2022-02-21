# Tila

Tila is a purely functional scripting language.

**Note**: _Tila is still in the design stage. It's not working yet, stay tuned..._ :-)

## Compiling
### Dependencies
- gcc
- glib

### Run make
`make` to create the binary in the source directory. Write your code to a file and execute the file with `./tila path/to/my/tila_file`.

## Examples
Define identifiers with `Define`
```
Define PI 3.1415 (every thing between parentheses is a comment)
Define E 2.7182 (define the Euler's number)
```
Use `pret` to show them on the screen
```
pret PI
pret E
```
will print
```
3.141500
2.718200
```

Functions are created with `Lambda`. A Lambda can have as many parameters as you like, but only one final expression which will be the value of the function when it is called. Lambda parameters are specified with a name and a trailing colon e.g. `PARAM:` or a name and a trailing colon followed by the equal sign `=` if the parameter should get a default argument, e.g. `PARAM:= PI`. 
```
Define print_number Lambda N:= PI pret N
```

To call a function use the `Call/X` keyword, where `X` is the number of arguments we want to pass to the function:
```
Call/0 print_number
Call/1 print_number E
```
will output:
```
3.141500
2.718200
```
We can also specify parameter names for arguments when calling a function:
```
Call/1 print_number N:= E
```
evaluates to
```
2.718200
```

Local bindings can be created by the `Let` keyword, followed by any number of bindings in the form `NAME:= <expr>` and a single final expression to which the Let evaluates. Below we define a closure function with zero arity and name it `print_year`, which prints the value of the local binding `YEAR` when called:

```
Let YEAR:= 2022 Define print_year Lambda pret YEAR
```
Calling `print_year`
```
Call/0 print_year
```
we get:
```
2022
```
Note that the binding `YEAR` exists only in the lexical environment of `print_year`:
```
pret YEAR
```
will thus output an error like:
```
lookup failed for
| | [tokstr(YEAR) id(18) type(Name) nadd(0x563de7412000) uadd(0x563de7409020) sz(0) atom(1) arity(-2) maxcap(0)]
```

***
© 2021-2022 Amir Teymuri
