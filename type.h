#ifndef LET_TYPE_H
#define LET_TYPE_H

enum _Type {
  NUMBER = 0, INTEGER = 1, FLOAT = 2,
  SYMBOL = 3, LAMBDA = 4,
  DOUBLE = 5,
  PARAMETER=6,
  BOUND_PARAMETER=7,	/* parameter with default argument */
  UNDEFINED
};
char *stringify_cell_type(enum _Type);
#endif	/* LET_TYPE_H */
