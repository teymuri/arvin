
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
enum __Type {
  NUM_T,
  LAMBDA_T
};
enum __Numtype { NAN, INT, FLT };
enum __Numtype numtype(char *str)
{
  bool dot = false;
  while (*str) {
    if (!isdig(*str)) {
      /* if a dot: set the type to float, but go on looking the rest
	 (which must be digit only to give a float!) */
      if (*str == '.') {
	dot = true;
	str++;		
      } else return NAN;	/* if not a digit and not a dot: Not A Number */
    } else str++;			/* if a digit: go on looking the rest */
  }
  return dot ? FLT : INT;
}

struct cell {
  /* evaluated values of CAR (caution! unset numbers are zero-initialized??) */
  int ival;
  float fval;
  enum __Type type;
  struct token car;
  struct cell *cdr;
};
/* only symbols will have envs!!! */
struct symbol {
  char *name;
  struct cell *(*lambda)(struct cell *);
};

int __Envid = 0;
struct env {
  struct symbol *names;		/* user defined */
  int id;
  struct env *parenv;		/* parent environment */
};

void resolve_self_eval(struct cell *cell)
{
  enum __Numtype t;
  if ((t = numtype(cell->car.str))) { /* it's a number */
    if (t == INT) {
      cell->ival = atoi(cell->car.str);
      cell->type = INT;
    }
    else { /* must be float */
      cell->fval = atof(cell->car.str);
      cell->type = FLT;
    }			
  }
}

/* This should be in a builtins.h or somethign like that! */
void _addition(struct cell *);

void eval(struct cell *sexp)
{
  switch (sexp->type) {
  case NUM_T:
    resolve_self_eval(sexp);
    /* if ((numtype((sexp->car).str)) == 1) { */
    /*   sexp->ival = atoi((sexp->car).str); */
    /*   printf("num %d \n", sexp->ival); */
    /* } */
    /* else if ((numtype((sexp->car).str)) == 2) /\* double *\/ */
    /*   sexp->fval = atof((sexp->car).str); */
    /* else fprintf(stderr, "numtype not supported"); */
    break;
  case LAMBDA_T:
    _addition(sexp);
  }
}

/* _apply(struct cell *(*f)(struct cell *), struct cell *args) */

void _addition(struct cell *sexp)
{
  struct cell *base = sexp;
  int i = 0;			/* ival of + */
  do {
    sexp = sexp->cdr;
    eval(sexp);
    i += sexp->ival;
    printf("%d %s %d\n", sexp->ival, sexp->car.str, i);
  } while (sexp->cdr != NULL);
  base->ival = i;
}

/* struct symbol __Builtins[] = { */
/*   struct symbol addition = {.name = "addition", .lambda = _addition} */
/* }; */
/* int __Builtins_count = 1; */
/* bool isbuiltin(char *name) */
/* { */
/*   for (int i = 0; i < __Builtins_count; i++) */
/*     if (!strcmp(name, __Builtins[i].name)) /\* found *\/ */
/*       return true; */
/*   return false; */
/* } */

/* struct cell *f(struct cell *i) */
/* { */
/*   struct cell *c = malloc(sizeof(struct cell)); */
/*   c->icar = 0; */
/*   c->cdr = NULL; */
/*   while (1) { */
/*     c->icar += i->icar; */
/*     if ((i->cdr == NULL)) break; */
/*     else i = i->cdr; */
/*   } */
/*   return c; */
/* } */

int main()
{
  size_t all_tokens_count = 0;
  struct token *toks = tokenize_source("/home/amir/a.let", &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments(toks, &nctok_count, all_tokens_count);

  /* for (size_t i = 0; i<nctok_count;i++) { */
  /*   printf("TOK-%zu. %s \n", i, nct[i].str); */
  /* } */

  struct cell b = {.car=nct[2], .cdr=NULL, .type=NUM_T};
  struct cell a = {.car=nct[1], .cdr=&b, .type=NUM_T};
  struct cell p = {.car=nct[0], .cdr=&a, .type=LAMBDA_T};
  eval(&p);
  printf("%d \n", p.ival);
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
