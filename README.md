# Tila

Tila is a purely functional scripting language.

**Note**: Tila is still in the design stage. It's not working yet. Stay tuned.

## Compiling
### Dependencies
- gcc
- glib

### Run make
`make`

### Examples
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
Define print_number Lambda num:= PI pret num
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
Call/1 print_number num:= E
```
evaluates to
```
2.718200
```
