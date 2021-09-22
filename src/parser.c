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
      } else return NAN;	/* if not a digit and not a dot: Not A Number */
    } else s++;			/* if a digit: go on looking the rest */
  }
  return dot ? FLOAT : INTEGER;
}

struct cell {
  struct token car;
  struct cell *cdr;
  /* evaluated values of CAR (caution! unset numbers are zero-initialized??) */
  int ival;
  float fval;
  enum __Type type;
};

typedef void (*let_lambda_t)(struct cell *);

/* only symbols will have envs!!! */
struct symbol {
  char *name;
  /* void (*lambda)(struct cell *); */
  let_lambda_t lambda;
};
/* This should be in a builtins.h or somethign like that! */
void _addition(struct cell *);
/* void add_lambda(let_lambda_t lam, struct symbol *lams, int *lams_count) */
/* { */
/*   lams + lams_count = struct symbol {.lambda=lam, .name="FOO"}; */
/* } */
int __Builtins_count = 1;
struct symbol __Builtins[] = {
  {.lambda=_addition, .name="_addition"}
};
int __Envid = 0;

struct env {
  int id;
  struct env *parenv;		/* parent environment */
};

/* *********** blocks start *********** */

int __Blockid = 0;
struct block {
  int id;
  struct cell head;
  /* struct token *body; */
  struct block *eblock;		/* enclosing block */
  struct env env;
};

bool is_cell_in_block(struct cell c, struct block b)
{
  return c.car.sidx > b.head.car.sidx && c.car.linum >= b.head.car.linum;
}

/* struct block __TLBlock = { */
/*   /\* id *\/ */
/*   0, */
/*   /\* toplevel cell *\/ */
/*   { */
/*     /\* toplevel cell's car (token) *\/ */
/*     {.str = TL_TOKEN_STR, */
/*      .sidx = -1, */
/*      .eidx = 100, */
/*      .linum = -1, */
/*      .id = 0 */
/*     } */
/*   }, */
/* }; */

/* ******************* blocks end ******************* */


void guess_type(struct token *t)
{
  enum __Type type;
  if ((type = (numtype(t->str)))) {
    if (type == INTEGER) {
      t->ival = atoi(t->str);
      t->type = INTEGER;
    } else {
      t->fval = atof(t->str);
      t->type = FLOAT;
    }
  }
}
/* { */
/*   switch (numtype(c->car.str)) { */
/*   case INTEGER: */
/*     c->ival = atoi(c->car.str); */
/*     c->type = INTEGER; */
/*     break; */
/*   case FLOAT: */
/*     c->fval = atof(c->car.str); */
/*     c->type = FLOAT; */
/*     break; */
/*   case NAN: */
/*     c->type = NAN; */
/*     break; */
/*   default: */
/*     break; */
/*   } */
/* } */


void eval(struct cell *sexp)
{
  switch (sexp->type) {
  case NUMBER:
    /* guess_type(sexp); */
    printf("type %d %s\n", sexp->type, sexp->car.str);
    break;
  case LAMBDA:
    _addition(sexp);
    break;
  default:
    break;
  }
}
struct cell *linked_cells(struct token tokens[], size_t count)
{
  struct cell *prev, *base;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct cell *c = malloc(sizeof(struct cell));
    if (i == 0) base = c;
    c->car = tokens[i];
    if (i > 0)
      prev->cdr = c;
    if (i == count-1)
      c->cdr = NULL;
    prev = c;
  }
  return base;
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

void _addition(struct cell *args)
{
  struct cell *base = args;
  int i = 0;		/* ival of + */
  do {
    args = args->cdr;
    eval(args);
    i += args->ival;
  } while (args->cdr != NULL);
  base->ival = i;
}

int main()
{
  size_t all_tokens_count = 0;
  struct token *toks = tokenize_source("/home/amir/a.let", &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments(toks, &nctok_count, all_tokens_count);
  
  /* for (size_t i = 0; i<nctok_count;i++) { */
  /*   printf("TOK-%zu. %s \n", i, nct[i].str); */
  /* } */

  /* struct cell b = {.car=nct[2], .cdr=NULL, .type=NUMBER}; */
  /* struct cell a = {.car=nct[1], .cdr=&b, .type=NUMBER}; */
  /* struct cell p = {.car=nct[0], .cdr=&a, .type=LAMBDA}; */
  /* eval(&p); */
  /* printf("%d \n", p.ival); */

  struct cell *c = linked_cells(nct, nctok_count);
  struct cell *base = c;
  while (1) {
    printf("cell %d -> %s\n", c->car.id, c->car.str);    
    if (c->cdr == NULL) break;
    else c = c->cdr;
  }
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
