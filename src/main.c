#include <stdlib.h>
#include <stdio.h>
#include "parser.h"

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"
int main()
{

  char *s = "(this is a comment)";
  int n =tokenize_line(s, TOKPATT, 0);
  printf("%d matches in %s\n", n, s);
  for (int i = 0; i < n; i++) {
    printf("** %s s:%d e:%d **\n", line_toks[i].str, line_toks[i].so, line_toks[i].eo);
  }

  /* size_t n = read_lines("/home/okavango/Work/let/etude.let"); */
  /* for (size_t i = 0; i<n; i++) */
  /*   printf("%zu %s", i, srclns[i]); */
  /* free_srclns(n); */
  exit(EXIT_SUCCESS);
}
