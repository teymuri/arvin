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

/* definer words */
#define NAME_GIVER_KW "name"		/* use to define symbols */

enum __Type {
  NUMBER, INTEGER, FLOAT,
  SYMBOL,
  UNDEFINED
};

char *stringize_type(enum __Type t)
{
  switch (t) {
  case 0: return "UNKNOWN/NUMBER?";
  case 1: return "Integer";
  case 2: return "Float";
  case 3: return "Symbol";
  default: return "Undefined";
  }
}

char *stringize_type(enum __Type);

#define MAX_TOKLEN 50		/* max token length in bytes */
#define TLTOKSTR "TLTOKSTR"

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int col_start_idx;			/* start index in line (column start index) */
  int col_end_idx;			/* end index in line (column end index) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comidx;			/* comment indices: 0 = (, 1 = ) */
  enum __Type type;		/* guessed types at token-generation time */
  /* int ival; */
  /* double fval; */
};

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMOP "("		/* comment opening token */
#define COMMCL ")"		/* comment closing token */




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
  if ((errcode = regcomp(&reint, "^[0-9]+$", REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &reint, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &reint, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
  }
  if ((errcode = regcomp(&refloat, "^[0-9]*\\.([0-9]*)?$", REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
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
      t.col_start_idx = offset + match[0].rm_so;
      t.col_end_idx = t.col_start_idx + tokstrlen;
      t.linum = linum;
      t.comidx = 0;
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


int iscom_open(struct token tok) {return !strcmp(tok.str, COMMOP);}
int iscom_close(struct token tok) {return !strcmp(tok.str, COMMCL);}

/* comment index 1 is the start of an outer-most comment block. this
   function is the equivalent of set_commidx_ip(toks) in the let.py
   file. */
void index_comments(struct token *tokens, size_t all_tokens_count)
{
  int idx = 1;
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (iscom_open(tokens[i]))
      tokens[i].comidx = idx++;
    else if (iscom_close(tokens[i]))
      tokens[i].comidx = --idx;
  }
}

struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count,
				  size_t all_tokens_count) /* nct = non-comment token */
{
  index_comments(toks, all_tokens_count);
  struct token *nctoks = NULL;	/* non-comment tokens */
  int isincom = false;		/* are we inside of a comment block? */
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (toks[i].comidx == 1) {
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


int isdig(char c)
{
  return ('0' <= c) && (c <= '9');
}

enum __Type numtype(char *s)
{
  bool dot = false;
  while (*s) {
    if (!isdig(*s)) {
      /* if a dot: set the type to float, but go on looking the rest
	 (which must be digit only to give a float!) */
      if (*s == '.') {
	dot = true;
	s++;		
      } else return UNDEFINED;	/* if not a digit and not a dot: Not A Number */
    } else s++;			/* if a digit: go on looking the rest */
  }
  return dot ? FLOAT : INTEGER;
}

/* /\* passing a token pointer to set it's fields *\/ */
/* void guess_token_type(struct token *t) */
/* { */
/*   enum __Type tp; */
/*   if ((tp = (numtype(t->str)))) { */
/*     if (tp == INTEGER) { */
/*       t->ival = atoi(t->str); */
/*       t->type = INTEGER; */
/*     } else if (tp == FLOAT) { */
/*       t->fval = atof(t->str); */
/*       t->type = FLOAT; */
/*     } */
/*   } else { */
/*     t->type = UNDEFINED; */
/*   } */
/* } */

enum __Block_content_type { CELL, BLOCK };
char *stringize_block_cont_type(enum __Block_content_type t)
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
  enum __Type type;
  struct block *enblock;	/* embedding block of this cell */
  struct cell *linker;		/* the cell linking into this cell */
  /* here will supported Let-types be stored as evaluating */
  int ival;
  float fval;
};

enum __Type celltype(struct cell *c)
{
  switch (c->car.type) {
  case INTEGER: return INTEGER;
  case FLOAT: return FLOAT;
    /* case SYMBOL: */
    /*   c->symval = makesym */
  default: return UNDEFINED;
  }
}

void set_cell_val(struct cell *c)
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
  GHashTable *symht;		/* hashtable keeping known symbols */
  int symcount;			/* number of symbols */
  struct env *parenv;		/* parent environment */

};

/* only symbols will have envs!!! */
struct symbol {
  int id;
  char *name;
  lambda_t lambda;		/* has a function value? */
  struct cell *cell;
};

struct symbol *make_symbol__Hp(struct cell *c, struct env *e)
{
  struct symbol *s = malloc(sizeof(struct symbol));
  s->name = c->car.str;
  g_hash_table_insert(e->symht, s->name, s);
  s->id = e->symcount++;
  return s;
}


/* 
name thing 2

*/

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

struct env *make_env__Hp(int id, struct env *parenv)
{
  struct env *e = malloc(sizeof(struct env));
  e->id = id;
  e->symht = g_hash_table_new(g_str_hash, g_str_equal); /* empty hashtable */
  e->symcount = 0;
  e->parenv = parenv;
  return e;
}


/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 1000

struct block_content {
  struct cell *c;		/* for a cell */
  struct block *b;		/* for a block */
  enum __Block_content_type type;
};

struct block {
  int id;
  struct cell cells[MAX_BLOCK_SIZE];
  struct env *env;
  int size;			/* number of cells contained in this block*/
  struct block_content *contents;		/* content cells & child blocks */
  /* the embedding block */
  struct block *enblock;
};

struct cell block_head(struct block *b) { return b->cells[0]; }


/* /\* Only blocks have evaluation types *\/ */
/* void set_eval_type(struct block *b) */
/* { */
/*   for (int i = 1; i < b.size; i++) {     */
/*   } */
/* } */

void add(struct block *b)
{
  float sum;
  for (int i = 1; i<b->size;i++) {
    switch (b->cells[i].car.type)
      {
      case INTEGER:
	sum += b->cells[i].ival;
	break;
      case FLOAT:
	sum += b->cells[i].fval;
	break;
      }
  }
  /* hardcoded */
  b->cells[0].type = FLOAT;
  b->cells[0].fval = sum;
}



/* is b directly or indirectly embedding c? */
bool is_enclosed_in(struct cell c, struct block b)
{
  return (c.car.col_start_idx > b.cells[0].car.col_start_idx) &&
    (c.car.linum >= b.cells[0].car.linum);
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
int botlinum(struct block **enblocks, int enblocks_count)
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
  int bln = botlinum(enblocks, enblocks_count);
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
  int col_start_idx = LEAST_COL_START_IDX;			/* start index */
  struct block *rmost_block = NULL;
  for (int i = 0; i < botmost_blocks_count; i++) {    
    if ((*(botmost_blocks + i))->cells[0].car.col_start_idx > col_start_idx) {
      rmost_block = *(botmost_blocks + i);
      col_start_idx = rmost_block->cells[0].car.col_start_idx;
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
    set_cell_val(c);
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
/*        .col_start_idx = -1, */
/*        .col_end_idx = 100,		/\* ???????????????????????????????????????? *\/ */
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
/*     /\* symht (GHashtable *), to be populated yet *\/ */
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

/* bool endswith(const char *str, const char *suffix) */
/* { */
/*   if (!str || !suffix) return 0; */
/*   size_t lenstr = strlen(str); */
/*   size_t lensuffix = strlen(suffix); */
/*   if (lensuffix > lenstr) return 0; */
/*   return !strcmp(str + lenstr - lensuffix, suffix); */
/* } */


bool need_new_block(struct cell *c, struct block *enblock)
{
  return isbuiltin(c)
    || !strcmp(c->car.str, NAME_GIVER_KW)
    || !strcmp(block_head(enblock).car.str, NAME_GIVER_KW);
}


struct block **parse__Hp(struct block *tlblock, struct cell *linked_cells_root, int *blocks_count)
{
  /* this is the blocktracker in the python prototype */
  struct block **blocks = malloc(sizeof(struct block *)); /* make room for the toplevel block */
  *(blocks + (*blocks_count)++) = tlblock;
  struct cell *c = linked_cells_root;
  struct block *enblock;
  int blockid = 1;  
  while (c) {
    /* find out the DIRECT embedding block of the current cell */
    enblock = enclosing_block(*c, blocks, *blocks_count);
    if (need_new_block(c, enblock)) {
      if ((blocks = realloc(blocks, (*blocks_count + 1) * sizeof(struct block *))) != NULL) {
	struct block *new_block = malloc(sizeof *new_block);
	new_block->id = blockid++;
	new_block->cells[0] = *c;
	new_block->size = 1;
	new_block->enblock = enblock;

	/* set the new block's content */
	new_block->contents = malloc(sizeof(struct block_content));
	(*(new_block->contents)).type = CELL;
	(*(new_block->contents)).c = c;
	
	*(blocks + (*blocks_count)++) = new_block;
	if ((enblock->contents = realloc(enblock->contents, (enblock->size+1) * sizeof(struct block_content))) != NULL) {
	  (*(enblock->contents + enblock->size)).type = BLOCK;
	  (*(enblock->contents + enblock->size)).b = new_block;
	  enblock->size++;
	}
      } else exit(EXIT_FAILURE); /* blocks realloc failed */      
    } else {			 /* no need for a new block, just a single lonely cell */
      if ((enblock->contents = realloc(enblock->contents, (enblock->size+1) * sizeof(struct block_content))) != NULL) {
	(*(enblock->contents + enblock->size)).type = CELL;
	(*(enblock->contents + enblock->size)).c = c;
      }
      c->enblock = enblock;
      enblock->cells[enblock->size] = *c;
      enblock->size++;
    }
    c = c->cdr;
  }
  return blocks;
}

void free_parser_blocks(struct block **blocks, int blocks_count)
{
  /* the first pointer in **blocks points to the tlblock, thats why we
     can't free *(blocks + 0), as tlblock is created on the
     stack in main(). that first pointer will be freed after the for loop. */
  for (int i = 1; i < blocks_count; i++) {
    free((*(blocks + i))->contents);
    free(*(blocks + i));
  }
  /* free the content of the toplevel block, since it surely
     containts something when the parsed string hasn't been an empty
     string! */
  free((*blocks)->contents);
  free(blocks);
}

void assign_envs(struct block **bs, int blocks_count, struct env *tlenv)
/* only blocks have envs??? */
{
  if (bs[0]->id == 0)		/* assert toplevel? */
    bs[0]->env = tlenv;  
  for (int i = 1; i < blocks_count; i++) {
    if (!strcmp(bs[i]->enblock->cells[0].car.str, NAME_GIVER_KW)
	/* || !strcmp(bs[i]->cells[0].car.str, NAME_GIVER_KW) */) {    
      bs[i]->env = tlenv;
      bs[i]->env->symcount++;
      /* g_hash_table_insert(bs[i]->env->symht, bs[i]->cells[0].car.str, &(bs[i]->cells[1])); */
      printf("***%d*****%s\n",bs[i]->size, bs[i]->cells[1].car.str);
      g_hash_table_insert(bs[i]->env->symht, bs[i]->cells[0].car.str, &(bs[i]->cells[1]));
    }
  }
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

#define AST_PRINTER_BLOCK_STR "[Block <HD %s> <SZ %d> <ENV(SZ %d, ID %d) %p>]\n"
#define AST_PRINTER_CELL_STR "[Cell %s <TYP %s>]\n"
void print_ast_code_part(struct block *root, int depth)
/* startpoint is the root block */
{
  for (int i = 0; i < root->size; i++) {
    switch (root->contents[i].type) {
    case CELL:
      print_indent(depth);
      printf(AST_PRINTER_CELL_STR,
	     root->contents[i].c->car.str,
	     /* stringize_type(root->contents[i].c->car.type) */
	     stringize_type(celltype(root->contents[i].c))
	     );
      break;
    case BLOCK:
      print_indent(depth);
      printf(AST_PRINTER_BLOCK_STR,
	     root->contents[i].b->cells[0].car.str,
	     root->contents[i].b->size,
	     root->contents[i].b->env ? root->contents[i].b->env->symcount : -1,
	     root->contents[i].b->env ? root->contents[i].b->env->id : -1,
	     root->contents[i].b->env ? (void *)root->contents[i].b->env : NULL
	     );
      print_ast_code_part(root->contents[i].b, depth+1);
      break;
    default:
      print_indent(depth);
      printf("[Invalid Content %d] %s %s\n", i,root[i].cells[0].car.str, root[i].contents[0].c->car.str);
    }
  }
}
void print_ast(struct block *root)
{
  /* root's (the toplevel block) contents is a block_content of
     type CELL, so when iterating over root's contents this CELL
     will be printed but there will be no BLOCK printed on top of that
     CELL, thats why we are cheating here and print a BLOCK-Like on
     top of the whole ast. */
  printf("----------------------------------\n");
  printf(AST_PRINTER_BLOCK_STR,
 	 root->contents->c->car.str,
 	 root->size,
	 root->env ? root->env->symcount : -1,
	 root->env ? root->env->id : -1,
	 root->env ? (void *)root->env : NULL);
  print_ast_code_part(root, 1);
  printf("----------------------------------\n");
}

/* void eval(struct block *root) */
/* { */
/*   for (int i = 0; i < root->size; i++) { */
/*     switch (root->contents[i].type) { */
/*     case CELL: */
/*       root->contents[i].c */
/*     } */
/*   } */
/* } */

#define X 2
int main()
{
  
  struct env tlenv = {
    .id = 0,
    .symht = g_hash_table_new(g_str_hash, g_str_equal),
    .parenv = NULL,
    .symcount = 0
  };
  struct token tltok = {
    .str = TLTOKSTR,
    .col_start_idx = -1,
    .col_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .linum = -1,
    .id = 0
  };
  struct cell tlcell = {				/* cells[0] toplevel cell */
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
  
  struct block_content *tlcont = malloc(sizeof (struct block_content));
  (*tlcont).type = CELL;
  (*tlcont).c = &tlcell;

  struct block tlblock = {
    .id = 0,
    .cells = { tlcell },
    /* env (Toplevel Environment) */
    .env = &tlenv,
    /* .env = NULL, */
    .size = 1,			/* this is the toplevel cell */
    .enblock = NULL,
    .contents = tlcont
  };

  
  /* struct block tlblock = { */
  /*   .id = 0, */
  /*   .cells = { */
  /*     {				/\* cells[0] toplevel cell *\/ */
  /* 	/\* car token *\/ */
  /* 	.car = { */
  /* 	  .str = TLTOKSTR, */
  /* 	  .col_start_idx = -1, */
  /* 	  .col_end_idx = 100,		/\* ???????????????????????????????????????? set auf maximum*\/ */
  /* 	  .linum = -1, */
  /* 	  .id = 0 */
  /* 	}, */
  /* 	/\* cdr cell pointer *\/ */
  /* 	.cdr = NULL, */
  /* 	/\* in block cdr *\/ */
  /* 	.in_block_cdr = NULL, */
  /* 	/\* type ??? *\/ */
  /* 	.type = UNDEFINED, */
  /* 	.linker = NULL,			/\* linker *\/ */
  /* 	.ival = 0,			/\* ival *\/ */
  /* 	.fval = 0.0			/\* fval *\/ */
  /*     } */
  /*   }, */
  /*   /\* env (Toplevel Environment) *\/ */
  /*   /\* .env=&tlenv, *\/ */
  /*   .env = NULL, */
  /*   .size = 1,			/\* this is the toplevel cell *\/ */
  /*   .enblock = NULL, */
  /*   .contents = NULL */
  /* }; */

  char *lines[X] = {
    "name pi 3.14",
    "name e 123453"
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
  struct block **b = parse__Hp(&tlblock, c, &blocks_count);
  assign_envs(b, blocks_count, &tlenv);
  /* print_ast_code_part(&tlblock, 0); */
  print_ast(&tlblock);
  
  /* printf("%s %d contsize %d\n", tlblock.cells[0].car.str, tlblock.size, tlblock.content_size); */
  /* printf("%s\n", ((struct block *)(tlblock.contents[1]))->cells[0].car.str); */
  /* printf("%s\n", ((struct block *)b[1])->cells[0].car.str); */
  /* printf("%s\n", ((struct cell *)(((struct block *)b[2])->contents[1]))->car.str); */
  
  /* printf("%s\n", ((tlcont[0])->contents[1])->cells[0].car.str); */
  /* for (int i = 0; i < blocks_count; i++) { */
  /*   printf("block id %d, sz %d head: [%s] EmblockHead: [%s/id:%d] Env [id:%d,sz:%d,?:%p]\n", */
  /* 	   b[i]->id, b[i]->size, b[i]->cells[0].car.str, */
  /* 	   b[i]->enblock ? b[i]->enblock->cells[0].car.str : "", */
  /* 	   /\* wo ist enblock NULL? enblock von tlblock!!! *\/ */
  /* 	   b[i]->enblock ? b[i]->enblock->id : -1, */
  /* 	   b[i]->env->id, */
  /* 	   b[i]->env->symcount, */
  /* 	   g_hash_table_lookup(b[i]->env->symht, "XXX") */
  /* 	   ); */
  /*   for (int j = 0; j<b[i]->size;j++) { */
  /*     printf(" Cell: str %s | toktype %s at %p val:%d\n", */
  /* 	     b[i]->cells[j].car.str, */
  /* 	     stringize_type(b[i]->cells[j].car.type), */
  /* 	     (void *)&(b[i]->cells[j]), */
  /* 	     b[i]->cells[j].ival */
  /* 	     ); */
  /*   } */
  /*   puts(""); */
  /* } */

  free_parser_blocks(b, blocks_count);
  
  free_linked_cells(base);
  
  free(nct);
    
  exit(EXIT_SUCCESS);


}
