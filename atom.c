#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include "token.h"
#include "type.h"
#include "atom.h"



/* why is this any good??? */
enum Type atom_type(struct Atom *c)
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

void determine_atom_type(struct Atom *c)
{
  switch (c->token.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case SYMBOL: c->type = SYMBOL; break;
  default: break;
  }
}

void determine_atom_value(struct Atom *c)
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
char *cellstr(struct Atom *c) {return c->token.str;}

/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/

GSList *units_linked_list(struct Token toks[], size_t toks_n)
{
  GSList *sll = NULL; /* the return singly linked list */
  struct Atom *atom = NULL;
  for (size_t s = 0; s < toks_n; s++) {
    /* struct Atom *atom = (struct Atom *)malloc(sizeof (struct Atom)); */
    atom = g_new(struct Atom, 1);
    /* uuid 0 is reserved for the toplevel atom, so start with 1 */
    atom->uuid = s+1;
    atom->token = toks[s];
    determine_atom_value(atom);
    determine_atom_type(atom);
    sll = g_slist_prepend(sll, atom);
  }
  sll = g_slist_reverse(sll);
  return sll;
}

/* returns a list of linked cells made of tokens */
struct Atom *linked_cells__Hp(struct Token tokens[], size_t count)
{
  struct Atom *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct Atom *c = malloc(sizeof (struct Atom));
    if (i == 0) root = c;
    
    /* guess_token_type(tokens+i);	/\* pass the pointer to the token *\/ */
    
    c->token = tokens[i];
    if (i > 0)
      prev->next = c;
    if (i == count-1)
      c->next = NULL;
    determine_atom_value(c);
    determine_atom_type(c);
    prev = c;
  }
  return root;
}

void free_linked_cells(struct Atom *c)
{
  struct Atom *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->next;
    free(tmp);
  }
}
