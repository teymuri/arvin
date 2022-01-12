#ifndef LET_LDATA_H
#define LET_LDATA_H

#include "type.h"
/* #include "lambda.h" */
/* #include "symbol.h" */

struct symbol;
struct lambda;

struct letdata {
  enum _Type type;
  union {
    int dataslot_int;
    float dataslot_float;
    struct symbol *dataslot_symbol;
    struct lambda *dataslot_lambda;
    /* struct letdata *(*fn)(); */
  } value;
};

#endif	/* LET_LDATA_H */
