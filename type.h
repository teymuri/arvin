#ifndef LET_TYPE_H
#define LET_TYPE_H

#include <glib.h>

enum Type {
  NUMBER = 0, INTEGER = 1, FLOAT = 2,
  NAME = 3, LAMBDA = 4,
  DOUBLE = 5,
  BINDING=6,
  BOUND_BINDING=7,	/* parameter with default argument */
  UNDEFINED
};


char *stringify_type(enum Type);

/* data structures of the landuage */

struct Lambda {
  GList *param_list;
  /* lambda environment is used at definition time  */
  GHashTable *env;
  GNode *node;			/* the lambda code */
};

struct Let_data {
  enum Type type;
  union {
    int int_slot;
    float float_slot;
    struct Lambda *slot_lambda;
  } data;
};


#endif	/* LET_TYPE_H */
