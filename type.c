
#include "type.h"
#include "plate_element.h"
#include "brick.h"
#include "plate.h"




char *stringify_block_item_type(enum Plate_element_type t)
{
  switch (t) {
  case BRICK:
    return "BRICK";
    break;
  case PLATE:
    return "PLATE";
    break;
  default:
    return "INVALID?????????????";
    break;
  }
}









/* löschen!!!!!!!! */
/* Type Lambda */
/* typedef void (*lambda_t)(struct Brick *); */







struct Brick block_head(struct Plate *b) { return *b->bricks[0]; }







/* char *stringify_cell_type(enum Type t) */
/* { */
/*   switch (t) { */
/*   case 0: return "Weiss net, waisch?!! vielleicht number??????"; */
/*   case 1: return "INTEGER"; */
/*   case 2: return "FLOAT"; */
/*   case 3: return "SYMBOL"; */
/*   case 4: return "LAMBDA"; */
/*   case 5: return "DOUBLE"; */
/*   case 6: return "BINDING"; */
/*   case 7: return "BOUND-BINDING"; */
/*   default: return "UNDEFINED"; */
/*   } */
/* } */


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
  case 7: return "boundbinding";
  default: return "undefined";
  }
}

