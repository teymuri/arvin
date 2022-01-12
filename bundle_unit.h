#ifndef LET_BUNDLE_UNIT_H
#define LET_BUNDLE_UNIT_H

#include "bit.h"
#include "bundle.h"

enum __Blockitem_type { CELL, BLOCK };

struct block_item {
  struct cell *cell_item;		/* for a cell */
  struct block *block_item;		/* for a block */
  enum __Blockitem_type type;
};


#endif	/* LET_BUNDLE_UNIT_H */
