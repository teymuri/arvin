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
  struct Brick **bricks;
  /* nicht alle Blocks brauchen eingenes env, z.B. +  */
  /* bool needs_env; */
  struct Environment *env;
  int size;			/* number of bricks contained in this block*/
  struct Plate_element *elements;		/* content bricks & child blocks */
  /* the embedding block */
  struct Plate *plate;
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
