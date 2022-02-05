#ifndef LET_UNIT_H
#define LET_UNIT_H

#include <stdbool.h>
#include <glib.h>
#include <stddef.h>
#include "type.h"
#include "token.h"

#define UNIT_FORMAT "[tokstr(%s) id(%zu) type(%s) nadd(%p) uadd(%p) sz(%d) atom(%d) arity(%d) maxcap(%d)]"

struct Unit {
  size_t uuid;
  /* arity is unsigned int for lambdas, and -1 otherwise (invalid
     arity if not a lambda) */
  int arity;
  GNode *lambda_expr;
  /* maximum absorption capacity: -1 = indefinite capacity, 0 = no
   capacity, else definite capacity*/
  int max_capa;
  struct Token token;
  enum Type type;
  bool is_atomic;
  GHashTable *env;	/* unit's environment */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};



enum Type unit_type(struct Unit *);

GList *brick_slist(struct Token tokens[], size_t count);
GList *unit_linked_list(struct Token toks[], size_t toks_n);
typedef struct Unit * unitp_t;
bool is_of_type(struct Unit *, enum Type);


#endif	/* LET_UNIT_H */
