#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"
int main()
{

  char *s = " let it  b(e89)";
  /* int n =tokenize_line(s, TOKPATT, 0); */
  int n = 0;
  struct token *t = tokenize_line2(s, TOKPATT, 10, &n);
  printf("%d matches in '%s'\n", n, s);
  for (int i = 0; i < n; i++, t++) {
    /* printf("** %s s:%d e:%d **\n", line_toks[i].str, line_toks[i].so, line_toks[i].eo); */
    printf("%d- %s s:%d, e:%d -\n", i, t->str, t->so, t->eo);
  }
  printf("=========\n");
  free(t-n);
  printf("=========\n");
  /* size_t n = read_lines("/home/okavango/Work/let/etude.let"); */
  /* for (size_t i = 0; i<n; i++) */
  /*   printf("%zu %s", i, srclns[i]); */
  /* free_srclns(n); */
  exit(EXIT_SUCCESS);
}
