#ifndef LET_DATA_H
#define LET_DATA_H

#include <glib.h>
#include "type.h"

struct Let_data {
  enum Type type;
  union {
    int dataslot_int;
    float dataslot_float;
    /* struct Symbol *name_slot; */
    GNode *lambda_slot;
  } value;
};

#endif	/* LET_DATA_H */
