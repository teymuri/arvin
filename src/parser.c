#define _GNU_SOURCE

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
#include "parser.h"



/*
 free srclns 
*/
void free_srclns(size_t n)
{
  for (size_t i = 0; i < n; i++)
    free(srclns[i]);
}

/* checks if the string s is only space or tab */
int isempty(char *s)
{
  while (*s) {
    /* if char is something other than a blank or a newline, the string
 is regarded as non-empty. */
    if (!isblank(*s) && *s != '\n')
      return 0;
    s++;
  }
  return 1;
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
    if (!isempty(lnptr)) {
      assert(("line count too large", count < MAXSRC));
      srclns[count++] = lnptr;
    }
    lnptr = NULL;
  }
  free(lnptr);
  fclose(stream);
  return count;
}
