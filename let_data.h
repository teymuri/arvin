#ifndef LET_DATA_H
#define LET_DATA_H

#include <glib.h>
#include "type.h"

/* data structures of the landuage */

struct Let_data {
  enum Type type;
  union {
    int int_slot;
    float float_slot;
    GNode *lambda_slot;
  } data;
};

#endif	/* LET_DATA_H */
