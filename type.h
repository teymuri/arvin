#ifndef LET_TYPE_H
#define LET_TYPE_H

enum Type {
  NUMBER = 0, INTEGER = 1, FLOAT = 2,
  NAME = 3, LAMBDA = 4,
  DOUBLE = 5,
  BINDING=6,
  BOUND_BINDING=7,	/* parameter with default argument */
  UNDEFINED
};


char *stringify_type(enum Type);



#endif	/* LET_TYPE_H */
