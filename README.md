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

The number of operands/expressions which are to be passed to an operator/form (`num_of_params` below) are indicated by a colon (`:`) followed by an integer right after the name of the operator/form (e.g. `Add:3` refers to the `Add` operator with three operands). Omitting this part implies `:0` (a _nullary_ operator/form). Thus both expressions `Lambda 1` and `Lambda:0 1` for instance specify a function which takes no arguments and returns 1 when called, and are hence equivalent.

#### Lambda
Any number of mandatory or optional parameters can be specified in any order.
The single __rest__ parameter (mandatory or optional) __must__ be the last parameter.  
```
Lambda<:num_of_params> [0,1]
  <mand_param> [0
  <@opt_param default_arg> [0
  <&mand_rest_param || @&opt_rest_param default_list_arg> [0,1]
  <expression> 1
```

#### Call
