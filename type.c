
#include "type.h"

#include "unit.h"

char *stringify_type(enum Type t)
{
  switch (t) {
  case 0: return "Weiss net, waisch?!! vielleicht number??????";
  case 1: return "integer";
  case 2: return "float";
  case 3: return "symbol";
  case 4: return "lambda";
  case 5: return "double";
  case 6: return "binding";
  case 7: return "bound-binding";
  default: return "undefined";
  }
}

