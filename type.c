
#include "type.h"
/* #include "const_item.h" */
#include "atom.h"
#include "cons.h"




/* char *stringify_block_item_type(enum Cons_item_type t) */
/* { */
/*   switch (t) { */
/*   case ATOM: */
/*     return "ATOM"; */
/*     break; */
/*   case CONS: */
/*     return "CONS"; */
/*     break; */
/*   default: */
/*     return "INVALID?????????????"; */
/*     break; */
/*   } */
/* } */









/* löschen!!!!!!!! */
/* Type Lambda */
/* typedef void (*lambda_t)(struct Atom *); */







struct Atom block_head(struct Cons *b) { return *b->bricks[0]; }







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

