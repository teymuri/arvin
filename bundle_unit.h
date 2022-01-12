#ifndef LET_BUNDLE_UNIT_H
#define LET_BUNDLE_UNIT_H

#include "bit.h"
#include "bundle.h"

enum Bundle_unit_type { CELL, BLOCK };

struct Bundle_unit {
  struct Bit *cell_item;		/* for a cell */
  struct Bundle *block_item;		/* for a block */
  enum Bundle_unit_type type;
};


#endif	/* LET_BUNDLE_UNIT_H */
