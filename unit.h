#ifndef LET_UNIT_H
#define LET_UNIT_H

#include <stdbool.h>
#include <glib.h>
#include <stddef.h>
#include "type.h"
#include "token.h"
#include "env.h"

struct Unit {
  size_t uuid;
  /* arity is unsigned int for lambdas, and -1 otherwise (invalid
     arity) */
  int arity;
  /* don't use unsigned for max absorption, since we need -1 for
     undefined capacity (or very big capacities!) */
  int max_capacity;
  struct Token token;
  enum Type type;
  bool is_atomic;
  struct Env *env;
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};


enum Type unit_type(struct Unit *);

GSList *brick_slist(struct Token tokens[], size_t count);
GSList *units_linked_list(struct Token toks[], size_t toks_n);
typedef struct Unit * unitp_t;
#endif	/* LET_UNIT_H */
