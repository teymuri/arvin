#ifndef LET_DATA_H
#define LET_DATA_H

#include "type.h"

struct Let_data {
  enum Type type;
  union {
    int dataslot_int;
    float dataslot_float;
    struct Symbol *dataslot_symbol;
    struct Lambda *dataslot_lambda;
  } value;
};

#endif	/* LET_DATA_H */
