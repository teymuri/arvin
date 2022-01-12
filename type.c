/* #include "let.h" */
#include "type.h"
#include "bundle_unit.h"
#include "bit.h"
#include "bundle.h"

char *stringify_cell_type(enum Type t)
{
  switch (t) {
  case 0: return "Weiss net, waisch?!! vielleicht number??????";
  case 1: return "INTEGER";
  case 2: return "FLOAT";
  case 3: return "SYMBOL";
  case 4: return "LAMBDA";
  case 5: return "DOUBLE";
  case 6: return "PARAMETER";
  case 7: return "BOUND_PARAMETER";
  default: return "UNDEFINED";
  }
}



char *stringify_block_item_type(enum Bundle_unit_type t)
{
  switch (t) {
  case CELL:
    return "CELL";
    break;
  case BLOCK:
    return "BLOCK";
    break;
  default:
    return "INVALID?????????????";
    break;
  }
}









/* löschen!!!!!!!! */
/* Type Lambda */
/* typedef void (*lambda_t)(struct Bit *); */







struct Bit block_head(struct Bundle *b) { return b->cells[0]; }









char *stringify_type(enum Type t)
{
  switch (t) {
  case 0: return "Weiss net, waisch?!! vielleicht number??????";
  case 1: return "INTEGER";
  case 2: return "FLOAT";
  case 3: return "SYMBOL";
  case 4: return "LAMBDA";
  default: return "UNDEFINED";
  }
}

