#include <stdlib.h>
#include <stdio.h>
#include "lexparse.h"


int main()
{

  size_t n = read_lines("/home/okavango/Work/let/etude.let");
  for (size_t i = 0; i<n; i++)
    printf("%zu %s", i, srclns[i]);
  free_srclns(n);
  exit(EXIT_SUCCESS);
}
