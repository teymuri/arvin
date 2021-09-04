#include <stdlib.h>
#include <stdio.h>
#include "parser.h"


int main()
{

  int n =tokenize_line("amir sd  neli pooran foo", "\\w+");
  printf("%d ---\n", n);
  for (int i = 0; i < n; i++) {
    printf("** %s **\n", lntoks[i]);
  }

  /* size_t n = read_lines("/home/okavango/Work/let/etude.let"); */
  /* for (size_t i = 0; i<n; i++) */
  /*   printf("%zu %s", i, srclns[i]); */
  /* free_srclns(n); */
  exit(EXIT_SUCCESS);
}
