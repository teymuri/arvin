#define _GNU_SOURCE
#include <stdlib.h>
#include <regex.h>
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
  for (size_t i = 0; i < n; i++) {
    free(srclns[i]);
    srclns[i] = NULL;
  }
}

/* checks if the string s consists only of blanks and/or newline */
int isempty(char *s, )
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

#define MAXSRC 100		/* max lines in a src */

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
    /* throw away empty lines */
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



struct token {
  /*
   * count: index of the token
   */
  int start, end, line, count;
  char *str;
};


#define MAXTOKLEN 50		/* bytes max token length */
#define MAXLNTOKS 10		/* max number of tokens in 1 line */
char lntoks[MAXLNTOKS][MAXTOKLEN];		/* tokens in 1 line */

int tokenize_line(char *line, const char *patt)
{
  regex_t re;
  int comperr;			/* compilation error */
  if ((comperr = regcomp(&re, patt, REG_EXTENDED))) {
    fprintf(stderr, "regcomp failed");
    exit(comperr);
  }
  regmatch_t matche[1];	/* interesed only in the whole match */
  int offset = 0;
  int i = 0, len;
  while (!regexec(&re, line + offset, 1, matche, REG_NOTBOL)) {
    len = matche[0].rm_eo - matche[0].rm_so;
    memcpy(lntoks[i], line + offset +matche[0].rm_so, len);
    lntoks[i++][len] = '\0';
    offset += matche[0].rm_eo;
  }
  regfree(&re);
  return i;
}
