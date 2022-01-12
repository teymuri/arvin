#ifndef LET_DATA_H
#define LET_DATA_H

#include "type.h"
/* #include "Lambda.h" */
/* #include "symbol.h" */


/* forward declare Symbol and Lambda structures to avoid cyclic
   dependency betwenn these and LetData */
/* struct Symbol; */
/* struct Lambda; */

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

#endif	/* LET_DATA_H */
