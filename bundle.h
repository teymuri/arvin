#ifndef LET_BUNDLE_H
#define LET_BUNDLE_H

#include <stdbool.h>
#include "bit.h"
#include "env.h"

struct Bit block_head(struct Bundle *b);

/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 10
struct Bundle {
  int id;
  /* struct Bit cells[MAX_BLOCK_SIZE]; */
  struct Bit **cells;
  /* nicht alle Blocks brauchen eingenes env, z.B. +  */
  /* bool needs_env; */
  struct Env *env;
  int size;			/* number of cells contained in this block*/
  struct Bundle_unit *items;		/* content cells & child blocks */
  /* the embedding block */
  struct Bundle *block_enclosing_block;
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
