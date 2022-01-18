#ifndef LET_CONS_ELEM_H
#define LET_CONS_ELEM_H

#include "atom.h"
#include "cons.h"


/* a construct item can be i single unit or a whole construct */

enum Cons_item_type { ATOM, CONS };


/* TODO: pack diese in ein union? */
struct Cons_item {
  struct Atom *the_unit;		/* for a cell */
  struct Cons *the_const;		/* for a block */
  enum Cons_item_type type;
};


#endif	/* LET_CONS_ELEM_H */
