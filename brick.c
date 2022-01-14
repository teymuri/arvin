
#include <stddef.h>
#include <stdlib.h>
#include "token.h"
#include "type.h"
#include "brick.h"



/* why is this any good??? */
enum Type celltype(struct Brick *c)
{
  switch (c->type) {
  case INTEGER: return INTEGER;
  case FLOAT: return FLOAT;
  case SYMBOL: return SYMBOL;
  case LAMBDA: return LAMBDA;
  case BINDING: return BINDING;
  case BOUND_BINDING: return BOUND_BINDING;
  default: return UNDEFINED;
  }
}

void set_cell_type(struct Brick *c)
{
  switch (c->car.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case SYMBOL: c->type = SYMBOL; break;
  default: break;
  }
}

void set_cell_value(struct Brick *c)
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
char *cellstr(struct Brick *c) {return c->car.str;}

/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/

/* returns a list of linked cells made of tokens */
struct Brick *linked_cells__Hp(struct Token tokens[], size_t count)
{
  struct Brick *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct Brick *c = malloc(sizeof (struct Brick));
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

void free_linked_cells(struct Brick *c)
{
  struct Brick *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->cdr;
    free(tmp);
  }
}
