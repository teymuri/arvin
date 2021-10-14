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
  struct cell *in_block_cdr;
  enum __Type type;
  struct block *emblock;	/* embedding block of this cell */
  struct cell *linker;		/* the cell linking into this cell */
};

char *cellstr(struct cell *c) {return c->car.str;}

typedef void (*lambda_t)(struct cell *);

struct env;

/* only symbols will have envs!!! */
struct symbol {
  int id;
  char *name;
  struct env *env;		/* symbol's environment */
  lambda_t lambda;		/* has a function value? */
  struct symbol *next;		/* order to use inside environment */
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




int __Envid = 0;

struct env {
  int id;
  struct symbol *syms;			/* in env known names */
  int syms_count;
  struct env *parenv;		/* parent environment */
};


/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 10
struct block {
  int id;
  struct cell cells[MAX_BLOCK_SIZE];
  int size;			/* number of cells */
};


/* is b directly or indirectly embedding c? */
bool is_embedded_in(struct cell c, struct block b)
{
  return (c.car.colsidx > b.cells[0].car.colsidx) && (c.car.linum >= b.cells[0].car.linum);
}


/* all blocks in which the cell is embedded. returns a pointer which
   points to pointers to block structures, so it's return value must
   be freed (which doesn't any harm to the actual structure pointers
   it points to!) */
struct block **embedding_blocks__Hp(struct cell c, struct block **blocks,
				   int bcount, int *eb_count)
{
  struct block **eb = NULL;
  for (int i = 0; i < bcount; i++)
    if (is_embedded_in(c, *blocks[i])) {
      if ((eb = realloc(eb, (*eb_count + 1) * sizeof(struct block *))) != NULL)
	*(eb + (*eb_count)++) = *(blocks + i);
      else exit(EXIT_FAILURE);
    }
  return eb;			/* free(eb) */
}

/* returns the max line number */
int bottomline(struct block **embedding_blocks, int eb_count)
{
  int ln = 0;
  for (int i = 0; i < eb_count; i++) {
    if ((*(embedding_blocks + i))->cells[0].car.linum > ln)
      ln = (*(embedding_blocks + i))->cells[0].car.linum;
  }
  return ln;
}

struct block **bottommost_blocks__Hp(struct block **embedding_blocks,
				     int eb_count, int *bmb_count)
{
  int bln = bottomline(embedding_blocks, eb_count);
  struct block **bmb = NULL;
  for (int i = 0; i < eb_count; i++) {
    if ((*(embedding_blocks + i))->cells[0].car.linum == bln) {
      if ((bmb = realloc(bmb, (*bmb_count + 1) * sizeof(struct block *))) != NULL) {
	*(bmb + (*bmb_count)++) = *(embedding_blocks +i);
      }	else exit(EXIT_FAILURE);
    }
  }
  /* free the pointer to selected (i.e. embedding) block pointers */
  free(embedding_blocks);
  return bmb;
}

static struct block *rightmost_block(struct block **bottommost_blocks, int bmb_count)
{
  int colsidx = -1;			/* start index */
  struct block *rmb;
  for (int i = 0; i < bmb_count; i++)
    if ((*(bottommost_blocks+i))->cells[0].car.colsidx > colsidx) {
      /* pick the hitherto rightmost block (note that the address is
	 coming from the original blocks in parse__Hp) */
      rmb = *(bottommost_blocks + i);
      colsidx = rmb->cells[0].car.colsidx;
    }
  free(bottommost_blocks);
  return rmb;
}

/* which one of the blocks is the direct embedding block of c? */
struct block *embedding_block(struct cell c, struct block **blocks, int bcount)
{
  int eb_count = 0;
  struct block **embedding_blocks = embedding_blocks__Hp(c, blocks, bcount, &eb_count);
  int bmb_count = 0;
  struct block **bottommost_blocks = bottommost_blocks__Hp(embedding_blocks, eb_count,
							  &bmb_count);
  return rightmost_block(bottommost_blocks, bmb_count);
}

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
  {				/* cells[] */
    {				/* cells[0] */
      /* car token */
      {.str = TLTOKSTR,
       .colsidx = -1,
       .coleidx = 100,
       .linum = -1,
       .id = 0
      },
      /* cdr cell pointer */
      NULL,
      /* in block cdr */
      NULL,
      /* type ??? */
      UNDEFINED,
      NULL,
      NULL
    }
  },
  /* /\* embedding block *\/ */
  /* NULL, */
  /* size */
  1
};
/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./tst 
*/

struct block **parse__Hp(struct cell *linked_cells_root, int *bcount)
{
  /* this is the blocktracker in the python prototype */
  struct block **blocks = malloc(sizeof(struct block *));
  *(blocks + (*bcount)++)  = &__TLBlock;
  struct cell *c = linked_cells_root;
  struct block *eblock;
  int bid = 1;			/* block id */

  while (c) {

    /* find out the direct embedding block of the current cell */
    eblock = embedding_block(*c, blocks, *bcount);
    if (isbuiltin(c)) {
      printf("* builtin %d %s\n", isbuiltin(c), cellstr(c));
      if ((blocks = realloc(blocks, (*bcount + 1) * sizeof(struct block *))) != NULL) {
	struct block *newb = malloc(sizeof *newb);
	newb->id = bid++;
	newb->cells[0] = *c;
	newb->size = 1;
	*(blocks + (*bcount)++) = newb;
      } else exit(EXIT_FAILURE);
    } else eblock->cells[eblock->size++] = *c;
    c = c->cdr;
  }
  return blocks;
}

void free_parser_blocks(struct block **blocks, int bcount)
{
  /* the first pointer in **blocks points to the tlblock, thats why we
     can't free *(blocks + 0). that first pointer will be freed after
     the loop. */
  for (int i = 1; i < bcount; i++) free(*(blocks + i));
  free(blocks);
}

int main()
{

  char *srclns[3] = {
    "+ 2 3 4",
    "  5 6 7",
    "* 10 + 20 30"
  };
  size_t all_tokens_count = 0;
  /* struct token *toks = tokenize_source__Hp("/home/amir/a.let", &all_tokens_count); */
  struct token *toks = tokenize_srclns__Hp(srclns, 3, &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments__Hp(toks, &nctok_count, all_tokens_count);
  
  /* for (size_t i = 0; i<nctok_count;i++) { */
  /*   printf("TOK-%zu. %s \n", i, nct[i].str); */
  /* } */

  struct cell *c = linked_cells__H(nct, nctok_count);
  struct cell *base = c;
  
  int bcount = 0;
  struct block **b = parse__Hp(c, &bcount);

  printf("=======================\nblock count: %d\n", bcount);
  for (int i = 0; i <bcount;i++) {
    printf("id %d, sz %d head[%s]\n", b[i]->id, b[i]->size, b[i]->cells[0].car.str);
    for (int j = 0; j<b[i]->size;j++)
      printf("  str %s\n", b[i]->cells[j].car.str);
  }
  
  free_parser_blocks(b, bcount);
  
  free_linked_cells(base);
  
  free(nct);
    
  exit(EXIT_SUCCESS);


}
