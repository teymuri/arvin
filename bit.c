
#include <stddef.h>
#include <stdlib.h>
#include "token.h"
#include "type.h"
#include "bit.h"



/* why is this any good??? */
enum Type celltype(struct Bit *c)
{
  /* switch (c->car.type) { */
  switch (c->type) {
  case INTEGER: return INTEGER;
  case FLOAT: return FLOAT;
  case SYMBOL: return SYMBOL;
  case PARAMETER: return PARAMETER;
  case BOUND_PARAMETER: return BOUND_PARAMETER;
  default: return UNDEFINED;
  }
}

void set_cell_type(struct Bit *c)
{
  switch (c->car.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case SYMBOL: c->type = SYMBOL; break;
  default: break;
  }
}

void set_cell_value(struct Bit *c)
{
  switch (c->car.type) {
  case INTEGER:
    c->ival = atoi(c->car.str);
    break;
  case FLOAT:
    c->fval = atof(c->car.str);
    break;
  default: break;
  }
}
char *cellstr(struct Bit *c) {return c->car.str;}

/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/

/* returns a list of linked cells made of tokens */
struct Bit *linked_cells__Hp(struct Token tokens[], size_t count)
{
  struct Bit *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct Bit *c = malloc(sizeof (struct Bit));
    if (i == 0) root = c;
    
    /* guess_token_type(tokens+i);	/\* pass the pointer to the token *\/ */
    
    c->car = tokens[i];
    if (i > 0)
      prev->cdr = c;
    if (i == count-1)
      c->cdr = NULL;
    set_cell_value(c);
    set_cell_type(c);
    prev = c;
  }
  return root;
}

void free_linked_cells(struct Bit *c)
{
  struct Bit *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->cdr;
    free(tmp);
  }
}
