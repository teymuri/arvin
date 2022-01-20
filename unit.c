#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include "token.h"
#include "type.h"
#include "unit.h"



/* why is this any good??? */
enum Type unit_type(struct Unit *c)
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

void determine_unit_type(struct Unit *c)
{
  switch (c->token.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case SYMBOL: c->type = SYMBOL; break;
  default: break;
  }
}

void determine_unit_value(struct Unit *c)
{
  switch (c->token.type) {
  case INTEGER:
    c->ival = atoi(c->token.str);
    break;
  case FLOAT:
    c->fval = atof(c->token.str);
    break;
  default: break;
  }
}


/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/

GSList *units_linked_list(struct Token toks[], size_t toks_n)
{
  GSList *sll = NULL; /* the return singly linked list */
  struct Unit *atom = NULL;
  for (size_t s = 0; s < toks_n; s++) {
    /* struct Unit *atom = (struct Unit *)malloc(sizeof (struct Unit)); */
    atom = g_new(struct Unit, 1);
    /* uuid 0 is reserved for the toplevel atom, so start with 1 */
    atom->uuid = s+1;
    atom->token = toks[s];
    determine_unit_value(atom);
    determine_unit_type(atom);
    sll = g_slist_prepend(sll, atom);
  }
  sll = g_slist_reverse(sll);
  return sll;
}

