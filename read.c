/*
if using glib compile with:

gcc -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -o /tmp/read read.c

*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
#include <glib.h>

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
#define ASSIGNMENT_KEYWORD "define"		/* used to define symbols in the global environment */
#define ASSOCIATION_KEYWORD "let"
#define LAMBDA_KW "lambda"
#define PARAM_PREFIX '.'
/* ********************************** */

/* Constructs beginning with an underscore and a capital are internal
   and may be modified or removed any time without notice... */

enum _Type {
  NUMBER = 0, INTEGER = 1, FLOAT = 2,
  SYMBOL = 3, LAMBDA = 4,
  DOUBLE = 5,
  PARAMETER=6,
  BOUND_PARAMETER=7,	/* parameter with default argument */
  UNDEFINED
};

char *stringify_cell_type(enum _Type t)
{
  switch (t) {
  case 0: return "Weiss net, waisch?!! vielleicht number??????";
  case 1: return "INTEGER";
  case 2: return "FLOAT";
  case 3: return "SYMBOL";
  case 4: return "LAMBDA";
  case 5: return "DOUBLE";
  case 6: return "PARAMETER";
  case 7: return "BOUND_PARAMETER";
  default: return "UNDEFINED";
  }
}

/* char *stringify_cell_type(enum _Type); */

#define MAX_TOKLEN 50		/* max token length in bytes */
#define TLTOKSTR "TLTOKSTR"

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int column_start_idx;			/* start index in line (column start index) */
  int column_end_idx;			/* end index in line (column end index) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comment_index;			/* comment indices: 0 = (, 1 = ) */
  enum _Type type;		/* guessed types at token-generation time */
};

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMENT_OPENING "("		/* comment opening token */
#define COMMENT_CLOSING ")"		/* comment closing token */




/* naming convention:
   global variables have 2 leading underscores and a Capital letter
*/

int __Tokid = 1;		/* id 0 is reserved for the toplevel
				   token */







/* checks if the string s consists only of blanks and/or newline */
int isempty(char *s)
{
  while (*s) {
    /* if char is something other than a blank or a newline, the string
       is regarded as non-empty. */
    if (!isblank(*s) && *s != '\n')
      return 0;
    s++;
  }
  return 1;
}



char **read_lines__Hp(char *path, size_t *count)
{
  FILE *stream;
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  char *lineptr = NULL;
  size_t n = 0;
  char **srclns = NULL;
  while ((getline(&lineptr, &n, stream) != -1)) {
    if (!isempty(lineptr)) {
      /* increment *count first, otherwise realloc will be called with size 0 :-O */
      if ((srclns = realloc(srclns, (*count + 1) * sizeof(char *))) != NULL) {
	*(srclns + (*count)++) = lineptr;
	lineptr = NULL;
      } else exit(EXIT_FAILURE);
    }
  }
  free(lineptr);
  fclose(stream);
  return srclns;
}

void free_lines(char **lines, size_t count)
{
  char **base = lines;
  while (count--) free(*lines++);
  free(base);
}

/* Generates tokens */
struct token *tokenize_line__Hp(char *line, size_t *line_toks_count, size_t *all_tokens_count, int linum)
{
  regex_t re;
  int errcode;			
  if ((errcode = regcomp(&re, TOKPATT, REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &re, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &re, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(errcode);
  }

  /* For type guessing */
  regex_t reint, refloat, resym;
  if ((errcode = regcomp(&reint, "^[-]*[0-9]+$", REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &reint, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &reint, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
  }
  if ((errcode = regcomp(&refloat, "^[-]*[0-9]*\\.([0-9]*)?$", REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &refloat, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &refloat, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
  }
  if ((errcode = regcomp(&resym, TOKPATT, REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &resym, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &resym, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
  }  
  regmatch_t match[1];	/* interesed only in the whole match */
  int offset = 0, tokstrlen;
  struct token *tokptr = NULL;
  /* overall size of memory allocated for tokens of the line sofar */
  size_t memsize = 0;
  /* int tokscnt = 0; */
  while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) { /* a match found */
    /* make room for the new token */
    memsize += sizeof(struct token);
    if ((tokptr = realloc(tokptr, memsize)) != NULL) { /* new memory allocated successfully */
      tokstrlen = match[0].rm_eo - match[0].rm_so;
      struct token t;
      memcpy(t.str, line + offset + match[0].rm_so, tokstrlen);
      t.str[tokstrlen] = '\0';

      /* guess type */
      if (!regexec(&reint, t.str, 0, NULL, 0)) {
	t.type = INTEGER;
      } else if (!regexec(&refloat, t.str, 0, NULL, 0)) {
	t.type = FLOAT;
      } else if (!regexec(&resym, t.str, 0, NULL, 0)) {
	t.type = SYMBOL;
      } else {
	/* fprintf(stderr, "couldn't guess type of token %s", t.str); */
	/* exit(EXIT_FAILURE); */
	t.type = UNDEFINED;
      }   
      /* t.numtype = numtype(t.str); */
      /* t.isprim = isprim(t.str); */
      t.id = __Tokid++;
      t.column_start_idx = offset + match[0].rm_so;
      t.column_end_idx = t.column_start_idx + tokstrlen;
      t.linum = linum;
      t.comment_index = 0;
      *(tokptr + *line_toks_count) = t;
      (*all_tokens_count)++;
      (*line_toks_count)++;
      offset += match[0].rm_eo;
    } else {
      fprintf(stderr, "realloc failed while tokenizing line %d at token %s", linum, "TOKEN????");
      /* just break out of executaion if haven't enough memory for the
	 next token. leave the freeing & cleanup over for the os! */
      exit(EXIT_FAILURE);
    }
  }
  regfree(&re);
  regfree(&reint);
  regfree(&refloat);
  regfree(&resym);
  return tokptr;
}


struct token *tokenize_source__Hp(char *path, size_t *all_tokens_count)
{
  size_t lines_count = 0;
  char **lines = read_lines__Hp(path, &lines_count);
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line__Hp */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line__Hp(lines[l], &line_toks_count, all_tokens_count, l);
    if ((tokens = realloc(tokens, *all_tokens_count * sizeof(struct token))) != NULL) {
      for (size_t i = 0; i < line_toks_count; i++) {
	*(tokens + i + global_toks_count_cpy) = lntoks[i];
      }
    } else {
      exit(EXIT_FAILURE);
    }
    free(lntoks);
    lntoks=NULL;
  }
  free_lines(lines, lines_count);
  return tokens;
}

struct token *tokenize_lines__Hp(char **srclns, size_t lines_count,
				  size_t *all_tokens_count)
{
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line__Hp */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line__Hp(srclns[l], &line_toks_count, all_tokens_count, l);
    if ((tokens = realloc(tokens, *all_tokens_count * sizeof(struct token))) != NULL) {
      for (size_t i = 0; i < line_toks_count; i++) {
	*(tokens + i + global_toks_count_cpy) = lntoks[i];
      }
    } else {
      exit(EXIT_FAILURE);
    }
    free(lntoks);
    lntoks=NULL;
  }
  return tokens;
}


int is_comment_opening(struct token tok) {return !strcmp(tok.str, COMMENT_OPENING);}
int is_comment_closing(struct token tok) {return !strcmp(tok.str, COMMENT_CLOSING);}

/* comment index 1 is the start of an outer-most comment block. this
   function is the equivalent of set_commidx_ip(toks) in the let.py
   file. */
void index_comments(struct token *tokens, size_t all_tokens_count)
{
  int idx = 1;
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (is_comment_opening(tokens[i]))
      tokens[i].comment_index = idx++;
    else if (is_comment_closing(tokens[i]))
      tokens[i].comment_index = --idx;
  }
}

struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count,
				  size_t all_tokens_count) /* nct = non-comment token */
{
  index_comments(toks, all_tokens_count);
  struct token *nctoks = NULL;	/* non-comment tokens */
  int isincom = false;		/* are we inside of a comment block? */
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (toks[i].comment_index == 1) {
      if (isincom) isincom = false;
      else isincom = true;
    } else if (!isincom) {
      /* not in a comment block, allocate space for the new non-comment token */
      /* (*nctok_count)++; */
      if ((nctoks = realloc(nctoks, ++(*nctok_count) * sizeof(struct token))) != NULL)
	/* the index for the new token is one less than the current number of non-comment tokens */
	*(nctoks + *nctok_count - 1) = toks[i];
      else
	exit(EXIT_FAILURE);
    }
  }
  free(toks);
  return nctoks;
}

enum __Blockitem_type { CELL, BLOCK };

char *stringify_block_item_type(enum __Blockitem_type t)
{
  switch (t) {
  case CELL:
    return "CELL";
    break;
  case BLOCK:
    return "BLOCK";
    break;
  default:
    return "INVALID";
    break;
  }
}
struct block;
struct symbol;

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


/* why is this any good??? */
enum _Type celltype(struct cell *c)
{
  /* switch (c->car.type) { */
  switch (c->type) {
  case INTEGER: return INTEGER;
  case FLOAT: return FLOAT;
  case SYMBOL: return SYMBOL;
  case PARAMETER: return PARAMETER;
  case BOUND_PARAMETER: return BOUND_PARAMETER;
  default: return UNDEFINED;
  }
}

void set_cell_type(struct cell *c)
{
  switch (c->car.type) {
  case INTEGER: c->type = INTEGER; break;
  case FLOAT: c->type = FLOAT; break;
  case SYMBOL: c->type = SYMBOL; break;
  default: break;
  }
}

void set_cell_value(struct cell *c)
{
  switch (c->car.type) {
  case INTEGER:
    c->ival = atoi(c->car.str);
    break;
  case FLOAT:
    c->fval = atof(c->car.str);
    break;
    /* case SYMBOL: */
    /*   c->symval = makesym */
  }
}
char *cellstr(struct cell *c) {return c->car.str;}


/* Type Lambda */
typedef void (*lambda_t)(struct cell *);


struct env {
  int id;
  GHashTable *hash_table;		/* hashtable keeping known symbols */
  /* int symcount;			/\* number of symbols *\/ */
  struct env *enclosing_env;		/* parent environment */
};


/* builtin functions */
char *__Builtins[] = {
  "+", "*", "-", "/"
};
int __Builtins_count = 4;

/* is the cell c a builtin? */
bool isbuiltin(struct cell *c)
{
  for (int i = 0; i < __Builtins_count; i++)
    if (!strcmp(cellstr(c), __Builtins[i]))
      return true;
  return false;
}


/* 
welche konstrukte generieren neue eigene envs?
* let
* lambda
 */

/* int __Envid = 0; */
/* 
name foo
       34
 */

/* struct env *make_env__Hp(int id, struct env *parenv) */
/* { */
/*   struct env *e = malloc(sizeof(struct env)); */
/*   e->id = id; */
/*   e->hashtable = g_hash_table_new(g_str_hash, g_str_equal); /\* empty hashtable *\/ */
/*   e->symcount = 0; */
/*   e->parenv = parenv; */
/*   return e; */
/* } */


/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 1000

/* ein block item kann ein Cell oder selbst ein Block sein (bestehend
   aus anderen block items)*/

struct block_item {
  struct cell *cell_item;		/* for a cell */
  struct block *block_item;		/* for a block */
  enum __Blockitem_type type;
};

/* struct block_item { */
/*   union { */
/*     struct cell *cell; */
/*     struct block *block; */
/*   } item; */
/*   enum __Blockitem_type type; */
/* }; */

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



struct cell block_head(struct block *b) { return b->cells[0]; }




/* is b directly or indirectly embedding c? */
bool is_enclosed_in(struct cell c, struct block b)
{
  return (c.car.column_start_idx > b.cells[0].car.column_start_idx)
    && (c.car.linum >= b.cells[0].car.linum);
}


/* all blocks in which the cell is embedded. returns a pointer which
   points to pointers to block structures, so it's return value must
   be freed (which doesn't any harm to the actual structure pointers
   it points to!) */
struct block **enclosing_blocks__Hp(struct cell c, struct block **blocks,
				    int blocks_count, int *enblocks_count)
{
  struct block **enblocks = NULL;
  for (int i = 0; i < blocks_count; i++) {
    if (is_enclosed_in(c, *(blocks[i]))) {
      if ((enblocks = realloc(enblocks, (*enblocks_count + 1) * sizeof(struct block *))) != NULL)
	*(enblocks + (*enblocks_count)++) = *(blocks + i);
      else exit(EXIT_FAILURE);
    }
  }
  return enblocks;
}

/* returns the bottom line number */
int bottom_line_number(struct block **enblocks, int enblocks_count)
{
  int ln = -1;
  for (int i = 0; i < enblocks_count; i++) {
    if ((*(enblocks + i))->cells[0].car.linum > ln)
      ln = (*(enblocks + i))->cells[0].car.linum;
  }
  return ln;
}

struct block **bottommost_blocks__Hp(struct block **enblocks, int enblocks_count, int *botmost_blocks_count)
{
  int bln = bottom_line_number(enblocks, enblocks_count);
  struct block **botmost_blocks = NULL;
  for (int i = 0; i < enblocks_count; i++) {
    if ((*(enblocks + i))->cells[0].car.linum == bln) {
      if ((botmost_blocks = realloc(botmost_blocks, (*botmost_blocks_count + 1) * sizeof(struct block *))) != NULL) {
	*(botmost_blocks + (*botmost_blocks_count)++) = *(enblocks + i);
      }	else exit(EXIT_FAILURE);
    }
  }
  /* free the pointer to selected (i.e. embedding) block pointers */
  free(enblocks);
  return botmost_blocks;
}


/* 
-2 can never exist as a starting index for a column the least existing
one ist -1 which belongs to the (invisible) toplevel block's head
cell.
 */
#define LEAST_COL_START_IDX -2
/* here we test column start index of block heads to decide */
static struct block *rightmost_block(struct block **botmost_blocks, int botmost_blocks_count)
{
  int column_start_idx = LEAST_COL_START_IDX;			/* start index */
  struct block *rmost_block = NULL;
  for (int i = 0; i < botmost_blocks_count; i++) {    
    if ((*(botmost_blocks + i))->cells[0].car.column_start_idx > column_start_idx) {
      rmost_block = *(botmost_blocks + i);
      column_start_idx = rmost_block->cells[0].car.column_start_idx;
    }
  }
  free(botmost_blocks);
  return rmost_block;
}

/* which one of the blocks is the direct embedding block of c? */
struct block *enclosing_block(struct cell c, struct block **blocks, int blocks_count)
{  
  int enblocks_count = 0;
  struct block **enblocks = enclosing_blocks__Hp(c, blocks, blocks_count, &enblocks_count);
  int botmost_blocks_count = 0;
  struct block **botmost_blocks = bottommost_blocks__Hp(enblocks, enblocks_count, &botmost_blocks_count);
  return rightmost_block(botmost_blocks, botmost_blocks_count);
}




/* returns a list of linked cells made of tokens */
struct cell *linked_cells__Hp(struct token tokens[], size_t count)
{
  struct cell *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct cell *c = malloc(sizeof(struct cell));
    if (i == 0) root = c;
    
    /* guess_token_type(tokens+i);	/\* pass the pointer to the token *\/ */
    
    c->car = tokens[i];
    if (i > 0)
      prev->cdr = c;
    if (i == count-1)
      c->cdr = NULL;
    set_cell_value(c);
    set_cell_type(c);
    prev = c;
  }
  return root;
}

/* struct cell *doubly_linked_cells(struct cell *c) */
/* { */
/*   struct cell *root = c;	/\* keep the address of root for return *\/ */
/*   struct cell *curr = NULL;		/\* current cell *\/ */
/*   while (c->cdr) { */
/*     c->linker = curr; */
/*     curr = c; */
/*     c = c->cdr; */
/*   } */
/*   c->linker = curr; */
/*   return root; */
/* } */

void free_linked_cells(struct cell *c)
{
  struct cell *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->cdr;
    free(tmp);
  }
}


/* struct block __TLBlock = { */
/*   /\* id *\/ */
/*   0, */
/*   /\* cells *\/ */
/*   { */
/*     {				/\* cells[0] *\/ */
/*       /\* car token *\/ */
/*       {.str = TLTOKSTR, */
/*        .column_start_idx = -1, */
/*        .column_end_idx = 100,		/\* ???????????????????????????????????????? *\/ */
/*        .linum = -1, */
/*        .id = 0 */
/*       }, */
/*       /\* cdr cell pointer *\/ */
/*       NULL, */
/*       /\* in block cdr *\/ */
/*       NULL, */
/*       /\* type ??? *\/ */
/*       UNDEFINED, */
/*       NULL,			/\* linker *\/ */
/*       .ival=0,			/\* ival *\/ */
/*       .fval=0.0			/\* fval *\/ */
/*     } */
/*   }, */
  
/*   /\* env (Toplevel Environment) *\/ */
/*   { */
/*     /\* id *\/ */
/*     0, */
/*     /\* hashtable (GHashtable *), to be populated yet *\/ */
/*     NULL, */
/*     /\* parenv *\/ */
/*     NULL, */
/*     /\* symcount *\/ */
/*     0 */
/*   }, */
  
/*   /\* size *\/ */
/*   1, */
/*   /\* child_blocks_count *\/ */
/*   0, */
/*   /\* child_blocks *\/ */
/*   NULL, */
/*   NULL */
/* }; */




/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/

/* bool looks_like_parameter(struct cell *c) */
/* { */
/*   return celltype(c) == SYMBOL && *c->car.str == PARAM_PREFIX; */
/* } */

/* whether string s1 ends with string s2? */
int str_ends_with(char *str1, char *str2)
{
  size_t len1 = strlen(str1);
  size_t len2 = strlen(str2);
  for (size_t i = 0; i<len2;i++) {
    if (!(str1[len1-(len2-i)] == str2[i])) {
      return 0;
    }
  }
  return 1;
}

bool looks_like_parameter(struct cell *c)
{
  return celltype(c) == SYMBOL && str_ends_with(c->car.str, ":");
}
bool looks_like_bound_parameter(struct cell *c)
{
  return celltype(c) == SYMBOL && str_ends_with(c->car.str, ":=");
}

bool is_lambda_head(struct cell c) { return !strcmp(c.car.str, LAMBDA_KW); }
bool is_association(struct cell *c)
{
  return !strcmp(c->car.str, ASSOCIATION_KEYWORD);
}

/* now we will be sure! */
bool is_parameter(struct cell *c, struct block *enclosing_block)
{
  return looks_like_parameter(c)
    && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).car.str,
						       ASSOCIATION_KEYWORD));
}
bool is_bound_parameter(struct cell *c, struct block *enclosing_block)
{
  return looks_like_bound_parameter(c)
    && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).car.str,
						       ASSOCIATION_KEYWORD));
}

bool is_define(struct block *b)
{
  return !strcmp(b->cells[0].car.str, ASSIGNMENT_KEYWORD);
}

/* is the direct enclosing block the bind keyword, ie we are about to
   define a new name? */
bool is_a_binding_name(struct block *b)
{
  return !strcmp(block_head(b->block_enclosing_block).car.str, ASSIGNMENT_KEYWORD);
}


bool need_new_block(struct cell *c, struct block *enclosing_block)
{
  return isbuiltin(c)
    || !strcmp(c->car.str, ASSIGNMENT_KEYWORD)
    || is_association(c)
    /* is the symbol to be defined? */
    /* || !strcmp(block_head(enclosing_block).car.str, ASSIGNMENT_KEYWORD) */
    /* is begin of a lambda expression? */
    || is_lambda_head(*c)
    /* is a lambda parameter? */
    || is_bound_parameter(c, enclosing_block) /* hier muss enclosing_block richtig entschieden sein!!! */
    || !strcmp(c->car.str, "call")
    || !strcmp(c->car.str, "pret")
    || !strcmp(c->car.str, "gj") /* geburtsjahr!!! */
    ;
}

struct block **parse__Hp(struct block *global_block, struct cell *linked_cells_root, int *blocks_count)
{
  /* this is the blocktracker in the python prototype */
  struct block **blocks = malloc(sizeof(struct block *)); /* make room for the toplevel block */
  *(blocks + (*blocks_count)++) = global_block;
  struct cell *c = linked_cells_root;
  struct block *enblock;
  struct block *active_superior_block;
  int blockid = 1;
  while (c) {
    
    /* find out the DIRECT embedding block of the current cell */
    if ((looks_like_parameter(c) || looks_like_bound_parameter(c)) && is_enclosed_in(*c, *active_superior_block)) { /* so its a lambda parameter */
      enblock = active_superior_block;
      active_superior_block->arity++;
      /* enhance the type of the parameter symbol. */
      /* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier,
	 denn is_bound_parameter fragt ab ob das Cell vom Typ SYMBOL
	 ist, was wiederum unten im need_new_block eine Rolle
	 spielt. Deshalb verschieben wir das Setzen vom Typ von SYMBOL zum
	 BOUND_PARAMETER auf nach need_new_block. */
      if (is_parameter(c, enblock)) c->type = PARAMETER;
      /* if (is_bound_parameter(c, enblock)) c->type = BOUND_PARAMETER; */
    } else {
      enblock = enclosing_block(*c, blocks, *blocks_count);
    }

    if (is_bound_parameter(enblock->cells + 0, enblock->block_enclosing_block)) {
      /* i.e. ist das Ding jetzt der WERT für einen Bound Parameter?
	 ich frag hier ob das enclosing Zeug von einem bound param
	 ist ...*/
      /* enblock ist hier der block von bound_param */
      if (enblock->max_absorption_capacity == 1) enblock->max_absorption_capacity = 0;
      else enblock = enblock->block_enclosing_block;
    }
    
    /* If the computed enclosing block is a lambda-parameter and it
       has no more absorption capacity then reset the enclosing block
       to be the enclosing block of the lambda-parameter block
       i.e. the lambda block itself (imply that the current item is
       the return-expression of the lambda-block). If the parameter
       block still has absorption capacity (i.e. it's single
       default-argument) the computed enclosing block is correct, only
       decrement it's absorption capacity. */
    /* &(enclosing_block->cells[0]) */
    
    if (need_new_block(c, enblock)) {
      if ((blocks = realloc(blocks, (*blocks_count + 1) * sizeof(struct block *))) != NULL) {

	struct block *newblock = malloc(sizeof *newblock);
	struct env *newenv = malloc(sizeof *newenv);
	newblock->id = blockid++;
	newblock->cells[0] = *c;
	newblock->size = 1;
	newblock->block_enclosing_block = enblock;
	*newenv = (struct env){
	  .enclosing_env = enblock->env,
	  .hash_table = g_hash_table_new(g_str_hash, g_str_equal)
	};
	newblock->env = newenv;

	/* set the new block's content */
	newblock->items = malloc(sizeof(struct block_item));
	(*(newblock->items)).type = CELL;
	(*(newblock->items)).cell_item = c;
	
	*(blocks + (*blocks_count)++) = newblock;
	
	/* keep an eye on this if its THE BEGINNING of a lambda */
	if (is_lambda_head(*c)) {
	  newblock->islambda = true; /* is a lambda-block */
	  newblock->arity = 0; /* default is null-arity */
	  active_superior_block = newblock;
	} else {
	  newblock->islambda = false;
	}
	/* LET Block */
	if (is_association(c)) {
	  active_superior_block = newblock; /* change this name */
	}

	if (is_bound_parameter(c, enblock)) {
	  newblock->max_absorption_capacity = 1;	/* ist maximal das default argument wenn vorhanden */
	  /* enhance the type from simple SYMBOL to BOUND_PARAMETER */
	  c->type = BOUND_PARAMETER;
	}
		
	/* das ist doppel gemoppelt, fass die beiden unten zusammen... */
	if ((enblock->items = realloc(enblock->items, (enblock->size+1) * sizeof(struct block_item))) != NULL) {
	  (*(enblock->items + enblock->size)).type = BLOCK;
	  (*(enblock->items + enblock->size)).block_item = newblock;
	  enblock->size++;
	}
      } else exit(EXIT_FAILURE); /* blocks realloc failed */      
    } else {			 /* no need for a new block, just a single lonely cell */
      if ((enblock->items = realloc(enblock->items, (enblock->size+1) * sizeof(struct block_item))) != NULL) {
	(*(enblock->items + enblock->size)).type = CELL;
	(*(enblock->items + enblock->size)).cell_item = c;
      }
      c->cell_enclosing_block = enblock;
      enblock->cells[enblock->size] = *c;
      enblock->size++;
    }
    c = c->cdr;
  }
  return blocks;
}

void free_parser_blocks(struct block **blocks, int blocks_count)
{
  /* the first pointer in **blocks points to the global_block, thats why we
     can't free *(blocks + 0), as global_block is created on the
     stack in main(). that first pointer will be freed after the for loop. */
  for (int i = 1; i < blocks_count; i++) {
    free((*(blocks + i))->items);
    free(*(blocks + i));
  }
  /* free the content of the toplevel block, since it surely
     containts something when the parsed string hasn't been an empty
     string! */
  free((*blocks)->items);
  free(blocks);
}

void print_indent(int i)
{
  int n = 2;
  char s[(i*n)+1];
  for (int k =0; k<i;k++) {
    s[k*n] = '|';
    s[(k*n)+1]=' ';
    /* s[(k*n)+2]=' '; */
  }
  s[(i*n)] = '\0';
  printf("%s", s);
}

#define AST_PRINTER_BLOCK_STR_TL "[!BLOCK HEAD(%s) SIZE(%d) ENV(SZ:%d ID:%d)%p ARITY(%d)]\n"
#define AST_PRINTER_BLOCK_STR "[BLOCK HEAD(%s) SIZE(%d) ENV(SZ:%d ID:%d)%p ARITY(%d)]\n"
#define AST_PRINTER_CELL_STR "[CELL(%s) TYPE(%s)]\n"
void print_code_ast(struct block *root, int depth) /* This is the written code part */
/* startpoint is the root block */
{
  for (int i = 0; i < root->size; i++) {
    switch (root->items[i].type) {
    case CELL:
      print_indent(depth);
      printf(AST_PRINTER_CELL_STR,
	     root->items[i].cell_item->car.str,
	     stringify_cell_type(celltype(root->items[i].cell_item))
	     );
      break;
    case BLOCK:
      print_indent(depth);
      printf(AST_PRINTER_BLOCK_STR,
	     root->items[i].block_item->cells[0].car.str,
	     root->items[i].block_item->size,
	     /* root->items[i].b->env ? root->items[i].b->env->symcount : -1, */
	     root->items[i].block_item->env ? -1 : -1,
	     root->items[i].block_item->env ? root->items[i].block_item->env->id : -1,
	     root->items[i].block_item->env ? (void *)root->items[i].block_item->env : NULL,
	     root->items[i].block_item->islambda ? root->items[i].block_item->arity : -1
	     );
      print_code_ast(root->items[i].block_item, depth+1);
      break;
    default:
      print_indent(depth);
      printf("[Invalid Content %d] %s %s\n", i,root[i].cells[0].car.str, root[i].items[0].cell_item->car.str);
    }
  }
}
void print_ast(struct block *root)
{
  /* root's (the toplevel block) items is a block_item of
     type CELL, so when iterating over root's items this CELL
     will be printed but there will be no BLOCK printed on top of that
     CELL, thats why we are cheating here and print a BLOCK-Like on
     top of the whole ast. */
  printf(AST_PRINTER_BLOCK_STR_TL,
 	 root->items->cell_item->car.str,
 	 root->size,
	 /* root->env ? root->env->symcount : -1, */
	 root->env ? -1 : -1,
	 root->env ? root->env->id : -1,
	 root->env ? (void *)root->env : NULL,
	 root->islambda ? root->arity : -1
	 );
  print_code_ast(root, 1);
}

struct lambda {
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
    struct lambda dataslot_lambda;
    /* struct letdata *(*fn)(); */
  } value;
};

char *stringify_type(enum _Type t)
{
  switch (t) {
  case 0: return "Weiss net, waisch?!! vielleicht number??????";
  case 1: return "INTEGER";
  case 2: return "FLOAT";
  case 3: return "SYMBOL";
  case 4: return "LAMBDA";
  default: return "UNDEFINED";
  }
}


/* string representation of data, this is the P in REPL */
/* data arg is the evaluated expression (is gone through eval already) */
void print(struct letdata *data)
{
  switch (data->type) {
  case INTEGER:
    printf("%d", data->value.dataslot_int);
    break;
  case FLOAT:
    printf("%f", data->value.dataslot_float);
    break;
  case LAMBDA:
    printf("lambda...");
    break;
  default:
    break;
  }
  puts("");
}


/* wie jede andere funktion, muss hier auch eine struct letdata pointer zurückgegeben werden */
/* das hier ist ein (interner Sprachkonstrukt) console.log ähnliches
   ding */
/* hoffentlich ist thing already evaled!!! */
struct letdata *__let_pret(struct letdata *thing)
{
  /* puts(">"); */
  switch(thing->type) {
  case INTEGER: printf("%d", thing->value.dataslot_int); break;
  case FLOAT: printf("%f", thing->value.dataslot_float); break;
  case LAMBDA: printf("tbi:lambda (to be implemented)"); break;
  default: break;
  }
  puts("");
  /* puts("<"); */
  return thing;
}

struct letdata *GJ(void) {
  struct letdata *ld = malloc(sizeof *ld);
  ld->type = INTEGER;
  ld->value.dataslot_int = 1363;
  /* printf("mein geburtsjahr %d", ld->value.i); */
  return ld;
}
/* struct letdata *(*lambda_0)(); */
/* struct letdata *(*f1)(struct letdata *); */
/* struct letdata *(*f2)(struct letdata *, struct letdata *); */
/* struct lambda { */
/*   int arity; */
/* blkcont} */


char *bound_parameter_name(char *param)
{
  char *name = malloc(strlen(param) - 1);
  for (size_t i = 0; i < (strlen(param) - 2); i++) {
    name[i] = param[i];
  }
  name[strlen(param) - 2] = '\0';
  return name;
}
/* eval evaluiert einen Baum */
struct letdata *eval__Hp(struct block_item *item,
			 struct env *local_env,
			 struct env *global_env)
{
  struct letdata *result = malloc(sizeof(struct letdata)); /* !!!!!!!!!! FREE!!!!!!!!!!!eval__Hp */
  switch (item->type) {
  case CELL:
    switch (celltype(item->cell_item)) {
    case INTEGER:
      result->type = INTEGER;
      result->value.dataslot_int = item->cell_item->ival;
      break;
    case FLOAT:
      result->type = FLOAT;
      result->value.dataslot_float = item->cell_item->fval;
      break;
    case SYMBOL:
      /* a symbol not contained in a BIND expression (sondern hängt einfach so rum im text) */
      {
	struct symbol *sym;
	char *symname = item->cell_item->car.str;
	/* struct symbol *sym = g_hash_table_lookup(local_env->hash_table, item->c->car.str); */
	struct env *e = local_env;
	while (e) {
	  if ((sym = g_hash_table_lookup(e->hash_table, symname))) {
	    result->type = sym->symbol_data->type;
	    switch (result->type) {
	    case INTEGER:
	      result->value.dataslot_int = sym->symbol_data->value.dataslot_int; break;
	    case FLOAT:
	      result->value.dataslot_float = sym->symbol_data->value.dataslot_float; break;
	    case LAMBDA:
	      result->value.dataslot_lambda = sym->symbol_data->value.dataslot_lambda; break;
	    default: break;
	    }
	    break;
	  } else {
	    e = e->enclosing_env;
	  }
	}
	if (!e) {		/* wir sind schon beim parent von global env angekommen */
	  fprintf(stderr, "unbound '%s'\n", symname);
	  exit(EXIT_FAILURE);
	}
      }
      break;
    default: break;
    }
    break;			/* break CELL */
  case BLOCK:
    if (!strcmp(block_head(item->block_item).car.str, LAMBDA_KW)) {
      result->type = LAMBDA; /* lambda objekte werden nicht in parse time generiert */
      struct lambda lambda;
      switch (item->block_item->arity) {
      case 0:
	/* wenn arity 0 ist, dann ist das nächste item gleich das return expression */
	lambda.return_expr = &(item->block_item->items[1]);
	result->value.dataslot_lambda = lambda;
	/* result->value.fn = &GJ; */
	/* result->value.fn = struct letdata *(*)(void); */
	break;
      }  
    } else if (!strcmp(block_head(item->block_item).car.str, "call")) {
      struct letdata *lambda_name_or_expr = eval__Hp(&(item->block_item->items[1]), local_env, global_env);
      result = eval__Hp(lambda_name_or_expr->value.dataslot_lambda.return_expr, local_env, global_env);
    } else if (!strcmp(block_head(item->block_item).car.str, "pret")) {
      result = __let_pret(eval__Hp(&((item->block_item)->items[1]), local_env, global_env));
    } else if (!strcmp(block_head(item->block_item).car.str, "gj")) {
      result = GJ();
      
    } else if (is_define(item->block_item)) { /* is_assignment */
      /* don't let the name of the binding to go through eval! */
      char *define_name = item->block_item->cells[1].car.str; /* name of the definition */
      /* data can be a lambda expr or some constant or other names etc. */
      struct letdata *define_data = eval__Hp(&(item->block_item->items[2]), local_env, global_env);
      struct symbol *sym= malloc(sizeof (struct symbol));
      sym->symbol_name = define_name;
      sym->symbol_data = define_data;
      /* definitions are always saved in the global environment, no
	 matter in which environment we are currently */
      g_hash_table_insert(global_env->hash_table, define_name, sym);
      result->type = SYMBOL;
      result->value.dataslot_symbol = sym;
      
    } else if (is_association(item->block_item->cells)) { /* = &(item->block_item->cells[0]) */
      /* add let parameters to it's hashtable */
      /* index 0 ist ja let selbst, fangen wir mit 1 an */
      for (int i = 1; i < item->block_item->size - 1;i++) {
	switch (item->block_item->items[i].type) {
	case CELL:		/* muss ein parameter ohne Wert sein, bind to NIL */
	  printf("%s %s\n",item->block_item->items[i].cell_item->car.str,
		 stringify_cell_type(celltype(item->block_item->items[i].cell_item)));
	  break;
	case BLOCK:		/* muss ein bound parameter sein! */
	  {
	    int bound_parameter_block_size = item->block_item->items[i].block_item->size;
	    assert(bound_parameter_block_size == 2);
	    char *parameter = item->block_item->items[i].block_item->cells[0].car.str;
	    /* char parameter_name[strlen(parameter)-1]; /\* jajaaaa VLA! *\/ */
	    /* /\* memset(parameter_name, '\0', sizeof (parameter_name)); *\/ */
	    /* strncpy(parameter_name, parameter, strlen(parameter)-2); */
	    /* /\* memcpy(parameter_name, parameter, strlen(parameter)-2); *\/ */
	    /* parameter_name[strlen(parameter)-2]='\0'; */
	    /* char *parameter_name = "x"; */
	    char *param_name = bound_parameter_name(parameter); /* problem mit strncpy */
	    struct letdata *parameter_data = eval__Hp(&(item->block_item->items[i].block_item->items[1]),
						      item->block_item->env,
						      global_env);
	    struct symbol *symbol = malloc(sizeof (struct symbol));
	    symbol->symbol_name = param_name;
	    symbol->symbol_data = parameter_data;
	    
	    g_hash_table_insert(item->block_item->env->hash_table, param_name, symbol);
	    break;
	  }
	  
	  /* { */
	  /*   if (i==item->block_item->size-1){ */
	  /*     result = eval__Hp(item->block_item->items + i, */
	  /* 			item->block_item->env, */
	  /* 			global_env); */
	  /*     break; */
	  /*   } else { */
	  /*     int bound_parameter_block_size = item->block_item->items[i].block_item->size; */
	  /*     assert(bound_parameter_block_size == 2); */
	  /*     char *parameter = item->block_item->items[i].block_item->cells[0].car.str; */
	  /*     /\* char parameter_name[strlen(parameter)-1]; /\\* jajaaaa VLA! *\\/ *\/ */
	  /*     /\* /\\* memset(parameter_name, '\0', sizeof (parameter_name)); *\\/ *\/ */
	  /*     /\* strncpy(parameter_name, parameter, strlen(parameter)-2); *\/ */
	  /*     /\* /\\* memcpy(parameter_name, parameter, strlen(parameter)-2); *\\/ *\/ */
	  /*     /\* parameter_name[strlen(parameter)-2]='\0'; *\/ */
	  /*     /\* char *parameter_name = "x"; *\/ */
	  /*     char *param_name = bound_parameter_name(parameter); /\* problem mit strncpy *\/ */
	  /*     struct letdata *parameter_data = eval__Hp(&(item->block_item->items[i].block_item->items[1]), */
	  /* 						item->block_item->env, */
	  /* 						global_env); */
	  /*     struct symbol *symbol = malloc(sizeof (struct symbol)); */
	  /*     symbol->symbol_name = param_name; */
	  /*     symbol->symbol_data = parameter_data; */
	    
	  /*     g_hash_table_insert(item->block_item->env->hash_table, param_name, symbol); */
	  /*     break; */
	  /*   } */
	  /* } */
	  
	}

    
      }
      result = eval__Hp(item->block_item->items + (item->block_item->size - 1),
			item->block_item->env,
			global_env);
	  

      
    }
    break;			/* break BLOCK */
  default: break;
  }
  return result;
}

struct letdata *global_eval(struct block *root,
			    struct env *local_env,
			    struct env *global_env)
{
  for (int i = 0; i < (root->size - 1); i++) {
    eval__Hp(&(root->items[i]), local_env, global_env);
  }
  return eval__Hp(&(root->items[root->size - 1]), local_env, global_env);
}




#define X 3
int main()
{
  /* The global environment */
  struct env global_env = {
    .id = 0,
    /* g_hash_table_new returns a GHashTable* */
    .hash_table = g_hash_table_new(g_str_hash, g_str_equal),
    .enclosing_env = NULL,
    /* .symcount = 0 */
  };

  struct token tltok = {
    .str = TLTOKSTR,
    .column_start_idx = -1,
    .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .linum = -1,
    .id = 0
  };
  struct cell global_cell = {				/* cells[0] toplevel cell */
    /* car token */
    .car = tltok,
    /* cdr cell pointer */
    .cdr = NULL,
    /* in block cdr */
    .in_block_cdr = NULL,
    /* type ??? */
    .type = UNDEFINED,
    .linker = NULL,			/* linker */
    .ival = 0,			/* ival */
    .fval = 0.0			/* fval */
  };
  
  struct block_item *global_item = malloc(sizeof(struct block_item));
  (*global_item).type = CELL;
  (*global_item).cell_item = &global_cell;

  struct block global_block = {
    .id = 0,
    .cells = { global_cell },
    /* env (Toplevel Environment) */
    .env = &global_env,
    /* .env = NULL, */
    .size = 1,			/* this is the toplevel cell */
    .block_enclosing_block = NULL,
    .items = global_item,
    .islambda = false,
    .arity = -1			/* invalid arity, because this is not a lambda block! */
  };

  char *lines[X] = {
    /* "call call lambda pret lambda pret gj" */
    
    /* "let y:= 3.14 y" */
    "let x:= 10 y:= 20 a:= 30",
    " amir:= 1363",
    "  pret y"
    /* "let y:= 7", */
    /* "  define scope-test", */
    /* "    lambda y" */
  };
  size_t all_tokens_count = 0;
  /* struct token *toks = tokenize_source__Hp("/home/amir/a.let", &all_tokens_count); */
  struct token *toks = tokenize_lines__Hp(lines, X, &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments__Hp(toks, &nctok_count, all_tokens_count);
  
  /* for (size_t i = 0; i<nctok_count;i++) { */
  /*   printf("TOK-%zu. %s \n", i, nct[i].str); */
  /* } */

  struct cell *c = linked_cells__Hp(nct, nctok_count);
  struct cell *base = c;
  int blocks_count = 0;
  struct block **b = parse__Hp(&global_block, c, &blocks_count);
  /* assign_envs(b, blocks_count, &global_env); */
  /* print_code_ast(&global_block, 0); */
  print_ast(&global_block);
  /* print(global_eval(&global_block, &global_env, &global_env)); */
  global_eval(&global_block, &global_env, &global_env);

  /* guint u = g_hash_table_size(global_env.hash_table); */
  /* gpointer* k=g_hash_table_get_keys_as_array(global_env.hash_table, &u); */
  /* for (guint i = 0; i < u;i++) { */
  /*   printf("KEY %s\n", (char *)k[i]); */
  /* } */


  
  free_parser_blocks(b, blocks_count);
  free_linked_cells(base);
  free(nct);
  exit(EXIT_SUCCESS);


}
