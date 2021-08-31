#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "let.h"


int main()
{

  size_t n = read_lines("/home/okavango/Work/let/etude.let");
  for (size_t i = 0; i<n; i++)
    printf("%zu %s", i, srclns[i]);
  free_srclns(n);
  exit(EXIT_SUCCESS);
}

/*
 free srclns 
*/
void free_srclns(size_t n)
{
  for (size_t i = 0; i < n; i++)
    free(srclns[i]);
}

size_t read_lines(char *path)
{
  FILE *stream;
  ssize_t read;
  char *lnptr = NULL;
  size_t n = 0;
  size_t count = 0;
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  while ((read = getline(&lnptr, &n, stream)) != -1) {
    /*
     * jump over the line if it begins with a newline char (dismissing empty lines)
     */
    if (*lnptr != '\n')
      srclns[count++] = lnptr;
    lnptr = NULL;
  }
  free(lnptr);
  fclose(stream);
  return count;
}
