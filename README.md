# Arvin

Arvin is a purely functional language.

**Warning**: Arvin is still in it's design stage and very much a work in progress. If you find errors don't grumble - [create an issue](https://github.com/ertpoi/arvin/issues) :-)


## Compiling
### Dependencies
- gcc
- glib

### Run make
`make` to create the binary in the source directory. Write your code to a file and execute the file with `./arvin path/to/my/arvin_script`.

## Syntax
The number of items are implied like 
- `1` exactly one
- `[0` 0 or more
- `[0,1] or [0,2)` 0 or 1

#### Lambda
Any number of mandatory or optional parameters can be specified in any order.
The single __rest__ parameter (mandatory or optional) __must__ be the last parameter.
```
Lambda:<N params> 1
  <mand_param> [0
  <@opt_param default_arg> [0
  <&mand_rest_param || @&opt_rest_param default_list_arg> [0,1]
  <expression> 1
```

#### Call