#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
// alle funktionen mussen ein cell * nehmen und eins zurückgeben!!!


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
struct block;
struct cell {
  struct token car;
  struct cell *cdr;
  enum __Type type;
  struct block *emblock;	/* embedding block of this cell */
  struct cell *linker;		/* the cell linking into this cell */
};

typedef void (*lambda_t)(struct cell *);
struct env;
/* only symbols will have envs!!! */
struct symbol {
  char *name;
  struct env *env;		/* symbol's environment */
  lambda_t lambda;		/* has a function value? */
};
/* This should be in a builtins.h or somethign like that! */
/* void _addition(struct cell *); */
/* void add_lambda(lambda_t lam, struct symbol *lams, int *lams_count) */
/* { */
/*   lams + lams_count = struct symbol {.lambda=lam, .name="FOO"}; */
/* } */
/* int __Builtins_count = 1; */
/* struct symbol __Builtins[] = { */
/*   {.lambda=_addition, .name="_addition"} */
/* }; */

int __Envid = 0;

struct env {
  int id;
  struct symbol *syms;			/* in env known names */
  int syms_count;
  struct env *parenv;		/* parent environment */
};
bool knowsym(struct symbol sym, struct env e)
{
  for (int i = 0; i < e.syms_count; i++)
    if (!strcmp(sym.name, e.syms[i].name))
      return true;
  return false;
}
/* *********** blocks start *********** */

int __Blockid = 0;

struct block {
  int id;
  struct cell head;
  /* struct token *body; */
  struct block *emblock;		/* embedding block */
  /* struct env env; */
};


/* is the block b directly or indirectly embedding the cell c? */
bool is_embedded_in(struct cell c, struct block b)
{
  return c.car.sidx > b.head.car.sidx && c.car.linum >= b.head.car.linum;
}


/* all blocks in which the cell is embedded */
struct block *embedding_blocks__H(struct cell c, struct block *blocks, int bcount, int *count)
{
  /* int count = 0; */
  struct block *bs = NULL;
  for (int i = 0; i < bcount; i++) {
    if (is_embedded_in(c, blocks[i])) {
      *count += 1;
      if ((bs = realloc(bs, *count * sizeof(struct block))) != NULL)
	bs[*count - 1] = blocks[i];
	/* *(bs + i) = blocks + i;	/\* store the address *\/ */
      else exit(EXIT_FAILURE);
    }
  }
  return bs;
}
/* returns the max line number */
int bottomline(struct block *embedding_blocks, int emb_count)
{
  int ln = 0;
  for (int i = 0; i < emb_count; i++)
    if (embedding_blocks[i].head.car.linum > ln)
      ln = embedding_blocks[i].head.car.linum;
  return ln;
}

struct block *bottommost_blocks__H(struct block *embedding_blocks, int emb_count, int *count)
{
  int bln = bottomline(embedding_blocks, emb_count);
  struct block *bs = NULL;
  /* int count = 0; */
  for (int i = 0; i < emb_count; i++)
    if ((embedding_blocks[i].head.car.linum == bln)) {
      *count += 1;
      if ((bs = realloc(bs, *count * sizeof(struct block))) != NULL)
	bs[*count - 1] = embedding_blocks[i];
	/* *(bs + i) = embedding_blocks + i; /\* store the address *\/ */
    }
  /* free_embedding_blocks(embedding_blocks); */
  return bs;
}

struct block *rightmost_block__H(struct block *bottommost_blocks, int btm_count)
{
  int sidx = 0;			/* start index */
  struct block *rightmost = malloc(sizeof(struct block));
  for (int i = 0; i < btm_count; i++)
    if (bottommost_blocks[i].head.car.sidx > sidx) {
      rightmost = bottommost_blocks + i;
      sidx = rightmost->head.car.sidx;
    }
  /* free_bottommost_blocks(bottommost_blocks); */
  return rightmost;
}


struct block *embedding_block(struct cell c, struct block *blocks, int bcount)
{
  int emb_count = 0;
  struct block *embedding_blocks = embedding_blocks__H(c, blocks, bcount, &emb_count);
  int btm_count = 0;
  struct block *bottommost_blocks = bottommost_blocks__H(embedding_blocks, emb_count, &btm_count);
  return rightmost_block__H(bottommost_blocks, btm_count);
}

/* ******************* blocks end ******************* */

/* passing a token pointer to set it's fields */
void guess_token_type(struct token *t)
{
  enum __Type tp;
  if ((tp = (numtype(t->str)))) {
    if (tp == INTEGER) {
      t->ival = atoi(t->str);
      t->type = INTEGER;
    } else if (tp == FLOAT) {
      t->fval = atof(t->str);
      t->type = FLOAT;
    }
  } else {
    t->type = UNDEFINED;
  }
}



/* void eval(struct cell *sexp) */
/* { */
/*   switch (sexp->type) { */
/*   case NUMBER: */
/*     /\* guess_token_type(sexp); *\/ */
/*     printf("type %d %s\n", sexp->type, sexp->car.str); */
/*     break; */
/*   case LAMBDA: */
/*     _addition(sexp); */
/*     break; */
/*   default: */
/*     break; */
/*   } */
/* } */


/* returns a list of linked cells made of tokens */
struct cell *linked_cells__H(struct token tokens[], size_t count)
{
  struct cell *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct cell *c = malloc(sizeof(struct cell));
    if (i == 0) root = c;
    guess_token_type(tokens+i);	/* pass the pointer to the token */
    c->car = tokens[i];
    if (i > 0)
      prev->cdr = c;
    if (i == count-1)
      c->cdr = NULL;
    prev = c;
  }
  return root;
}

struct cell *double_linked_cells(struct cell *c)
{
  struct cell *root = c;	/* festhalten! */
  struct cell *crnt = NULL;		/* current */
  int i = 0;
  while (c->cdr) {
    c->linker = crnt;
    crnt = c;
    c = c->cdr;
  }
  c->linker = crnt;
  return root;
}
void free_linked_cells(struct cell *c)
{
  struct cell *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->cdr;
    free(tmp);
  }
}
/*
 ----------     -----------
|    |  *--|-->|    | NULL |
 ----------     -----------
*/
/* _apply(struct cell *(*f)(struct cell *), struct cell *args) */

/* void _addition(struct cell *args) */
/* { */
/*   struct cell *base = args; */
/*   int i = 0;		/\* ival of + *\/ */
/*   do { */
/*     args = args->cdr; */
/*     eval(args); */
/*     i += args->ival; */
/*   } while (args->cdr != NULL); */
/*   base->ival = i; */
/* } */
struct block __TLBlock = {
  /* id */
  0,
  /* head cell */
  {
    /* car token */
    {.str = TLTOKSTR,
     .sidx = -1,
     .eidx = 100,
     .linum = -1,
     .id = 0
    },
    /* cdr cell pointer */
    NULL,
    /* type ??? */
    UNDEFINED,
    NULL,
    NULL
  },
  /* embedding block */
  NULL
};
bool islex(struct cell c)
{
  static char *lexical_block_builders[] = {
    "name", "lambda"
  };
  static int count = 2;
  for (int i = 0; i < count; i++)
    if (!strcmp(c.car.str, lexical_block_builders[i]))
      return true;
  return false;
}

/* void append_cell(struct cell *c, struct block *b) */
/* { */
/*   while (b->head != NULL) { */
/*     b->head.cdr = b->head.cdr->cdr; */
/*   } */
/*   b->head.cdr = c; */
/* } */

struct block *ast__H(struct cell *cells)
{
  /* this is the blocktracker in the python prototype parse */
  struct block *blocks = malloc(sizeof(struct block));
  blocks[0] = __TLBlock;
  int bcount = 1;
  struct cell *c = cells;
  int bidx = 1;
  while (c) {			/* c is cell pointer??? */
    printf("> %s\n", c->car.str);
    /* find out the embedding block of this cell */
    struct block *emblock = embedding_block(*c, blocks, bcount);

    if (!strcmp(c->car.str, "+")) { /* new block */
      /* unlink_last_cell(blocks[bidx - 1], bcount); */
      if ((blocks = realloc(blocks, bidx+1 * sizeof(struct block))) != NULL) {
	struct block B;
	B.head = *c;
      } else exit(EXIT_FAILURE);
      /* else there must already be a block for this */
    } else {
      
    }
    c = c->cdr;
  }
  return blocks;
}

int main()
{
  size_t all_tokens_count = 0;
  struct token *toks = tokenize_source__H("/home/amir/a.let", &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments__H(toks, &nctok_count, all_tokens_count);
  
  /* for (size_t i = 0; i<nctok_count;i++) { */
  /*   printf("TOK-%zu. %s \n", i, nct[i].str); */
  /* } */

  /* struct cell b = {.car=nct[2], .cdr=NULL, .type=NUMBER}; */
  /* struct cell a = {.car=nct[1], .cdr=&b, .type=NUMBER}; */
  /* struct cell p = {.car=nct[0], .cdr=&a, .type=LAMBDA}; */
  /* eval(&p); */
  /* printf("%d \n", p.ival); */

  struct cell *c = double_linked_cells(linked_cells__H(nct, nctok_count));
  struct cell *base = c;
  /* struct block *_ast = ast__H(c); */
  while (1) {
    printf("cell %d -> %s [type] %s [linker] %s [CDR] %s\n", c->car.id, c->car.str, stringize_type(c->car.type), c->linker->car.str, c->cdr->car.str);
    if (c->cdr == NULL) break;
    else c = c->cdr;
  }

  /* printf("%s\n", _ast->head.car.str); */
  /* free(_ast); */
  free_linked_cells(base);  
  free(nct);
    
  exit(EXIT_SUCCESS);

  /* //struct cell *(*p)(struct cell *a); */
  /* struct symbol s = {.lambda = f, .name="addition"}; */
  /* struct cell z = {.icar=3, .cdr=NULL}; */
  /* struct cell b ={.icar =10, .cdr=&z}; */
  /* struct cell a = {.icar =4, .cdr =&b}; */
  /* struct cell *ant =s.lambda(&b); /\* lambda member funktioniert! *\/ */
  /* printf("%s %d\n", s.name, ant->icar); */

  /* free(ant); */
  /* return 0; */
}
