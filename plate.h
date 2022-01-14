#ifndef LET_MULTIPLE_H
#define LET_MULTIPLE_H

#include <stdbool.h>
#include "brick.h"
#include "env.h"

struct Brick block_head(struct Plate *b);

/* int __Blockid = 0; */
#define MAX_PLATE_SIZE 10
struct Plate {
  int id;
  /* struct Brick cells[MAX_PLATE_SIZE]; */
  struct Brick **cells;
  /* nicht alle Blocks brauchen eingenes env, z.B. +  */
  /* bool needs_env; */
  struct Env *env;
  int size;			/* number of cells contained in this block*/
  struct Plate_element *items;		/* content cells & child blocks */
  /* the embedding block */
  struct Plate *block_enclosing_block;
  bool islambda;
  void *(*lambda)(void *);
  int arity;			/* arity is the number of arguments
				   and exists only for lambda-blocks,
				   dont confuse it with absoption
				   capacity which exists in ANY block!!! */
  int max_absorption_capacity;		/* wieviel Zeug wird in diesem block
				   max reingesteckt werden können?! */
};

#endif	/* LET_MULTIPLE_H */
