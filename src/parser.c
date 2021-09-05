#define _GNU_SOURCE
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
#include "parser.h"

#define MAXSRC 100		/* max lines in a src */
char *srclns[MAXSRC];	/* source lines add max line length*/

void free_srclns(size_t n)
{
  for (size_t i = 0; i < n; i++) {
    free(srclns[i]);
    srclns[i] = NULL;
  }
}

/* checks if the string s consists only of blanks and/or newline */
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
  int start, end, line, idx;
  char *str;
};


#define TOK_MAX_LEN 50		/* bytes max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */
char line_toks[MAX_LINE_TOKS][TOK_MAX_LEN];		/* tokens in 1 line */

/* char *get_regerror(int errcode, regex_t *compiled) */
/* { */
/*   size_t len = regerror(errcode, compiled, NULL, 0); */
/*   char *buff = malloc(len); */
/*   (void)regerror(errcode, compiled, buff, len); */
/*   return buff; */
/* } */

int tokenize_line(char *line, const char *patt)
{
  regex_t re;
  int errcode;			
  if ((errcode = regcomp(&re, patt, REG_EXTENDED))) { /* compilation failed */
    size_t buff_size = regerror(errcode, &re, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &re, buff, buff_size);
    fprintf(stderr, "PARSE ERROR\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(errcode);
  }
  regmatch_t match[1];	/* interesed only in the whole match */
  int offset = 0, count = 0, len;
  while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) {
    len = match[0].rm_eo - match[0].rm_so;
    memcpy(line_toks[count], line + offset +match[0].rm_so, len);
    line_toks[count++][len] = '\0';
    offset += match[0].rm_eo;
  }
  regfree(&re);
  return count;
}
