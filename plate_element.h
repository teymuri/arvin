#ifndef LET_PLATE_ELEMENT_H
#define LET_PLATE_ELEMENT_H

#include "brick.h"
#include "plate.h"

enum Plate_element_type { BRICK, PLATE };

struct Plate_element {
  struct Brick *brkelt;		/* for a cell */
  struct Plate *pltelt;		/* for a block */
  enum Plate_element_type type;
};


#endif	/* LET_PLATE_ELEMENT_H */
