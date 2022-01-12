#ifndef LET_BIT_H
#define LET_BIT_H

#include <stddef.h>
#include "type.h"
#include "token.h"

struct cell {
  struct token car;
  struct cell *cdr;
  struct cell *in_block_cdr;
  enum _Type type;
  struct block *cell_enclosing_block;	/* embedding block of this cell */
  struct cell *linker;		/* the cell linking into this cell */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};
struct cell *linked_cells__Hp(struct token tokens[], size_t count);
void free_linked_cells(struct cell *c);
enum _Type celltype(struct cell *);
char *cellstr(struct cell *);
#endif	/* LET_BIT_H */
