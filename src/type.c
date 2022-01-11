#include "let.h"

char *stringify_cell_type(enum _Type t)
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



char *stringify_block_item_type(enum __Blockitem_type t)
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






/* why is this any good??? */
enum _Type celltype(struct cell *c)
{
  /* switch (c->car.type) { */
  switch (c->type) {
  case INTEGER: return INTEGER;
  case FLOAT: return FLOAT;
  case SYMBOL: return SYMBOL;
  case PARAMETER: return PARAMETER;
  case BOUND_PARAMETER: return BOUND_PARAMETER;
  default: return UNDEFINED;
  }
}

void set_cell_type(struct cell *c)
{
  switch (c->car.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case SYMBOL: c->type = SYMBOL; break;
  default: break;
  }
}

void set_cell_value(struct cell *c)
{
  switch (c->car.type) {
  case INTEGER:
    c->ival = atoi(c->car.str);
    break;
  case FLOAT:
    c->fval = atof(c->car.str);
    break;
  default: break;
  }
}
char *cellstr(struct cell *c) {return c->car.str;}

/* löschen!!!!!!!! */
/* Type Lambda */
/* typedef void (*lambda_t)(struct cell *); */







struct cell block_head(struct block *b) { return b->cells[0]; }









char *stringify_type(enum _Type t)
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

