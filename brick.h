#ifndef LET_BRICK_H
#define LET_BRICK_H

#include <stddef.h>
#include "type.h"
#include "token.h"

struct Brick {
  struct Token car;
  struct Brick *cdr;
  struct Brick *in_block_cdr;
  enum Type type;
  struct Plate *cell_enclosing_block;	/* embedding block of this cell */
  struct Brick *linker;		/* the cell linking into this cell */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};
struct Brick *linked_cells__Hp(struct Token tokens[], size_t count);
void free_linked_cells(struct Brick *c);
enum Type celltype(struct Brick *);
char *cellstr(struct Brick *);
#endif	/* LET_BRICK_H */
