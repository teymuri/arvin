#ifndef LET_DATA_H
#define LET_DATA_H

#include <glib.h>
#include "type.h"

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

#endif	/* LET_DATA_H */
