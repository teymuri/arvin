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
? 0 or 1
* 0 or more

# Lambda
```
Lambda:<number of parameters>
  mand_params*
  @opt_param <default arg>*
  &mand_rest_param?
  @&opt_rest_param <default list>?
  <expression>
```

# Call