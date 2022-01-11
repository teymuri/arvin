#ifndef __TYPES_H
#define __TYPES_H

#include <stdbool.h>
#include <glib.h>


#define TLTOKSTR "TLTOKSTR"
/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
#define ASSIGNMENT_KEYWORD "define"		/* used to define symbols in the global environment */
#define ASSOCIATION_KEYWORD "let"
#define LAMBDA_KW "lambda"
#define PARAM_PREFIX '.'


enum _Type {
  NUMBER = 0, INTEGER = 1, FLOAT = 2,
  SYMBOL = 3, LAMBDA = 4,
  DOUBLE = 5,
  PARAMETER=6,
  BOUND_PARAMETER=7,	/* parameter with default argument */
  UNDEFINED
};

#define MAX_TOKLEN 50		/* max token length in bytes */
struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int column_start_idx;			/* start index in line (column start index) */
  int column_end_idx;			/* end index in line (column end index) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comment_index;			/* comment indices: 0 = (, 1 = ) */
  enum _Type type;		/* guessed types at token-generation time */
};


char *stringify_cell_type(enum _Type);
enum __Blockitem_type { CELL, BLOCK };
char *stringify_block_item_type(enum __Blockitem_type);

struct symbol;
struct block;

struct cell {
  struct token car;
  struct cell *cdr;
  struct cell *in_block_cdr;
  enum _Type type;
  struct block *cell_enclosing_block;	/* embedding block of this cell */
  struct cell *linker;		/* the cell linking into this cell */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};
/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 1000
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


enum _Type celltype(struct cell *);
void set_cell_type(struct cell *c);
void set_cell_value(struct cell *c);
char *cellstr(struct cell *c);
struct env {
  int id;
  GHashTable *hash_table;		/* hashtable keeping known symbols */
  /* int symcount;			/\* number of symbols *\/ */
  struct env *enclosing_env;		/* parent environment */
};
/* ein block item kann ein Cell oder selbst ein Block sein (bestehend
   aus anderen block items)*/

struct block_item {
  struct cell *cell_item;		/* for a cell */
  struct block *block_item;		/* for a block */
  enum __Blockitem_type type;
};

struct cell block_head(struct block *b);
struct cell block_head(struct block *b);

struct lambda {
  struct env *lambda_env;
  struct block_item *return_expr;	/* the return statement */
};

struct letdata;
/* only symbols will have envs!!! */
struct symbol {
  char *symbol_name;
  struct letdata *symbol_data;
};

struct letdata {
  enum _Type type;
  union {
    int dataslot_int;
    float dataslot_float;
    struct symbol *dataslot_symbol;
    struct lambda *dataslot_lambda;
    /* struct letdata *(*fn)(); */
  } value;
};

char *stringify_type(enum _Type t);

#endif
