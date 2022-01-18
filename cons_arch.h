#ifndef LET_CONS_ITEM_H
#define LET_CONS_ITEM_H

#include "atom.h"
#include "plate.h"

enum Cons_item_type { BRICK, PLATE };

struct Cons_item {
  struct Atom *brick_element;		/* for a cell */
  struct Cons *pltelt;		/* for a block */
  enum Cons_item_type type;
};


#endif	/* LET_CONS_ITEM_H */
