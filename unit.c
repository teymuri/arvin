#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include "token.h"
#include "type.h"
#include "unit.h"



/* why is this any good??? */
enum Type unit_type(struct Unit *u) {
  switch (u->type) {
  case INTEGER: return INTEGER;
  case FLOAT: return FLOAT;
  case NAME: return NAME;
  case LAMBDA: return LAMBDA;
  case BINDING: return BINDING;
  case BOUND_BINDING: return BOUND_BINDING;
  case PACK_BINDING: return PACK_BINDING;
  case BOUND_PACK_BINDING: return BOUND_PACK_BINDING;
  case PACK: return PACK;
  default: return UNDEFINED;
  }
}

void determine_unit_type(struct Unit *c)
{
  switch (c->token.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case NAME: c->type = NAME; break;
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

GList *unit_linked_list(struct Token toks[], size_t toks_n)
{
  GList *link = NULL; /* the return singly linked list */
  struct Unit *unit = NULL;
  for (size_t s = 0; s < toks_n; s++) {
    unit = g_new(struct Unit, 1);
    unit->env = NULL;
    /* unit->lambda_env = g_hash_table_new(g_str_hash, g_str_equal); */
    /* uuid 0 is reserved for the toplevel unit, so start with 1 */
    unit->uuid = s + 1;
    unit->token = toks[s];
    /* maximum absorption of -1 means undefined, i.e. can take as much
       as you want. This will be reset later in parser to possinly
       different unsigned integers for different types (e.g. parameter
       bindings will get 1 etc.)*/
    unit->max_capa = -1;
    /* arity will be set by the parser to either 0 or more for lambdas
       and remains -1 for everything else */
    unit->arity = -1;
    determine_unit_value(unit);
    determine_unit_type(unit);
    link = g_list_prepend(link, unit);
  }
  link = g_list_reverse(link);
  return link;
}

