#ifndef __LET_H
#define __LET_H


#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <glib.h>



/* #include <stdbool.h> */
#include <regex.h>
/* #include <stdio.h> */
#include <ctype.h>
/* #include <stdlib.h> */



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

extern bool isbuiltin(struct cell *);
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

/* ast.c */
bool is_enclosed_in(struct cell c, struct block b);
struct block **enclosing_blocks__Hp(struct cell c, struct block **blocks,
				    int blocks_count, int *enblocks_count);
int bottom_line_number(struct block **enblocks, int enblocks_count);
struct block **bottommost_blocks__Hp(struct block **enblocks, int enblocks_count, int *botmost_blocks_count);
#define LEAST_COL_START_IDX -2
struct block *rightmost_block(struct block **botmost_blocks, int botmost_blocks_count);
struct block *enclosing_block(struct cell c, struct block **blocks, int blocks_count);
int str_ends_with(char *str1, char *str2);
bool looks_like_parameter(struct cell *c);
bool looks_like_bound_parameter(struct cell *c);
bool is_lambda_head(struct cell c);
bool is_association(struct cell *c);
bool is_parameter(struct cell *c, struct block *enclosing_block);
bool is_bound_parameter(struct cell *c, struct block *enclosing_block);
bool is_define(struct block *b);
bool is_a_binding_name(struct block *b);
bool need_new_block(struct cell *c, struct block *enclosing_block);
struct block **parse__Hp(struct block *global_block, struct cell *linked_cells_root, int *blocks_count);
void free_parser_blocks(struct block **blocks, int blocks_count);

/* core */
struct letdata *pret(struct letdata *thing);
struct letdata *GJ(void);
extern char *__Builtins[];
int __Builtins_count;
bool isbuiltin(struct cell *c);
/* eval */
char *bound_parameter_name(char *param);
struct letdata *eval__dynmem(struct block_item *item,
			 struct env *local_env,
			     struct env *global_env);
struct letdata *global_eval(struct block *root,
			    struct env *local_env,
			    struct env *global_env);
/* print */
#define AST_PRINTER_BLOCK_STR_TL "[!BLOCK HEAD(%s) SIZE(%d) ENV(SZ:%d ID:%d)%p ARITY(%d)]\n"
#define AST_PRINTER_BLOCK_STR "[BLOCK HEAD(%s) SIZE(%d) ENV(SZ:%d ID:%d)%p ARITY(%d)]\n"
#define AST_PRINTER_CELL_STR "[CELL(%s) TYPE(%s)]\n"
void print_indent(int i);
void print_code_ast(struct block *root, int depth);
void print_ast(struct block *root);
void print(struct letdata *data);
/* read */
#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMENT_OPENING "("		/* comment opening token */
#define COMMENT_CLOSING ")"		/* comment closing token */

int __Tokid;
int isempty(char *s);
char **read_lines__Hp(char *path, size_t *count);
void free_lines(char **lines, size_t count);
struct token *tokenize_line__Hp
(char *line, size_t *line_toks_count, size_t *all_tokens_count, int linum);
struct token *tokenize_source__Hp(char *path, size_t *all_tokens_count);
struct token *tokenize_lines__Hp(char **srclns, size_t lines_count,
				 size_t *all_tokens_count);
int is_comment_opening(struct token tok);
int is_comment_closing(struct token tok);
void index_comments(struct token *tokens, size_t all_tokens_count);
struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count,
				  size_t all_tokens_count);
struct cell *linked_cells__Hp(struct token tokens[], size_t count);
void free_linked_cells(struct cell *c);

/* type */
char *stringify_cell_type(enum _Type t);
char *stringify_block_item_type(enum __Blockitem_type t);
enum _Type celltype(struct cell *c);
void set_cell_type(struct cell *c);
void set_cell_value(struct cell *c);
char *cellstr(struct cell *c);
typedef void (*lambda_t);
struct cell block_head(struct block *b);


#endif
