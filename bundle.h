#ifndef LET_BUNDLE_H
#define LET_BUNDLE_H

#include <stdbool.h>
#include "bit.h"
#include "env.h"

struct cell block_head(struct block *b);

/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 10
struct block {
  int id;
  struct cell cells[MAX_BLOCK_SIZE];
  /* nicht alle Blocks brauchen eingenes env, z.B. +  */
  /* bool needs_env; */
  struct env *env;
  int size;			/* number of cells contained in this block*/
  struct block_item *items;		/* content cells & child blocks */
  /* the embedding block */
  struct block *block_enclosing_block;
  bool islambda;
  void *(*lambda)(void *);
  int arity;			/* arity is the number of arguments
				   and exists only for lambda-blocks,
				   dont confuse it with absoption
				   capacity which exists in ANY block!!! */
  int max_absorption_capacity;		/* wieviel Zeug wird in diesem block
				   max reingesteckt werden können?! */
};

#endif	/* LET_BUNDLE_H */
