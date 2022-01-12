#ifndef LET_LETDATA_H
#define LET_LETDATA_H

#include "type.h"
/* #include "Lambda.h" */
/* #include "Symbol.h" */

struct Symbol;
struct Lambda;

struct LetData {
  enum Type type;
  union {
    int dataslot_int;
    float dataslot_float;
    struct Symbol *dataslot_symbol;
    struct Lambda *dataslot_lambda;
    /* struct letdata *(*fn)(); */
  } value;
};

#endif	/* LET_LETDATA_H */
