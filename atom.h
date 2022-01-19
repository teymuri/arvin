#ifndef LET_ATOM_H
#define LET_ATOM_H

#include <stdbool.h>
#include <glib.h>
#include <stddef.h>
#include "type.h"
#include "token.h"
#include "env.h"

struct Atom {
  size_t uuid;
  size_t arity;			/* falls ein lambda... */
  size_t max_absorption_capacity;
  struct Token token;
  struct Atom *next;
  struct Atom *in_block_cdr;
  enum Type type;
  /* enum Unit_type unit_t;	/\* This will be going to be an ATOM *\/ */
  /* /\* alternative to unit_t: *\/ */
  /* /\* ein child das selbst keine kinder haben kann *\/ */
  bool is_atomic;
  struct Env *env;
  struct Cons *enclosure;	/* embedding block of this cell */
  struct Atom *parent_unit;
  struct Atom *linker;		/* the cell linking into this cell */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};
struct Atom *linked_cells__Hp(struct Token tokens[], size_t count);
void free_linked_cells(struct Atom *c);
enum Type atom_type(struct Atom *);
char *cellstr(struct Atom *);
GSList *brick_slist(struct Token tokens[], size_t count);
GSList *units_linked_list(struct Token toks[], size_t toks_n);
typedef struct Atom * unitp_t;
#endif	/* LET_ATOM_H */
