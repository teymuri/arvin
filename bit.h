#ifndef LET_BIT_H
#define LET_BIT_H

#include <stddef.h>
#include "type.h"
#include "token.h"

struct Bit {
  struct Token car;
  struct Bit *cdr;
  struct Bit *in_block_cdr;
  enum Type type;
  struct Bundle *cell_enclosing_block;	/* embedding block of this cell */
  struct Bit *linker;		/* the cell linking into this cell */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};
struct Bit *linked_cells__Hp(struct Token tokens[], size_t count);
void free_linked_cells(struct Bit *c);
enum Type celltype(struct Bit *);
char *cellstr(struct Bit *);
#endif	/* LET_BIT_H */
