
#include "type.h"

#include "unit.h"

char *stringify_type(enum Type t)
{
  switch (t) {
  case 0: return "Weiss net, waisch?!! vielleicht number??????";
  case 1: return "INTEGER";
  case 2: return "FLOAT";
  case 3: return "SYMBOL";
  case 4: return "LAMBDA";
  case 5: return "DOUBLE";
  case 6: return "BINDING";
  case 7: return "BOUND-BINDING";
  default: return "UNDEFINED";
  }
}

