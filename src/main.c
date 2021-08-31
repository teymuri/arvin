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
  char *lnptr;
  size_t n;
  size_t count;
  stream = fopen(path, "r");
  lnptr = NULL;
  n = 0;
  count = 0;
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  while ((read = getline(&lnptr, &n, stream)) != -1) {
    /*
     * strlen returns number of chars (excluding the end \0 byte). \n
     * is a char, dismiss only a single char line when the char is \n
     * (on a line might be a single non-\n char which we need to read)
     */
    if (*lnptr != '\n')
      srclns[count++] = lnptr;
    lnptr = NULL;
  }
  free(lnptr);
  fclose(stream);
  return count;
}
