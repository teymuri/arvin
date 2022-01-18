#ifndef LET_CONS_H
#define LET_CONS_H

#include <stdbool.h>
#include <glib.h>
#include "atom.h"
#include "env.h"


struct Atom block_head(struct Cons *b);

/* int __Blockid = 0; */
#define MAX_CONS_SIZE 10
struct Cons {
  int id;
  /* enum Unit_type unit_t;    /\* This is a CONS *\/ */
  /* struct Atom cells[MAX_CONS_SIZE]; */
  struct Atom **bricks;	/* cells */
  GSList *brks;		/* cells2 */
  GSList *atoms;
  /* nicht alle Blocks brauchen eingenes env, z.B. +  */
  /* bool needs_env; */
  struct Env *env;
  int size;			/* number of bricks contained in this block*/
  /* old name: items */
  struct Cons_item *elts;		/* content bricks & child blocks */
  GSList *items;
  /* the embedding block */
  struct Cons *enclosure;
  bool islambda;
  void *(*lambda)(void *);
  int arity;			/* arity is the number of arguments
				   and exists only for lambda-blocks,
				   dont confuse it with absoption
				   capacity which exists in ANY block!!! */
  int max_absr_capa;		/* wieviel Zeug wird in diesem block
				   max reingesteckt werden können?! */
};

#endif	/* LET_CONS_H */
