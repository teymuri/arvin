#define _GNU_SOURCE
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
#include "parser.h"




void free_srclns(size_t n)
{
  for (size_t i = 0; i < n; i++) {
    free(src_lines[i]);
    src_lines[i] = NULL;
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

int number_of_lines = 0;

/* size_t read_lines(char *path) */
/* { */
/*   FILE *stream; */
/*   ssize_t read; */
/*   char *lnptr = NULL; */
/*   size_t n = 0; */
/*   size_t count = 0; */
/*   stream = fopen(path, "r"); */
/*   if (!stream) { */
/*     fprintf(stderr, "can't open source '%s'\n", path); */
/*     exit(EXIT_FAILURE); */
/*   } */
/*   while ((read = getline(&lnptr, &n, stream)) != -1) { */
/*     /\* throw away empty lines *\/ */
/*     if (!isempty(lnptr)) { */
/*       assert(("line count too large", count < MAX_SRC_LINES)); */
/*       src_lines[count++] = lnptr; */
/*     } */
/*     lnptr = NULL; */
/*   } */
/*   free(lnptr); */
/*   fclose(stream); */
/*   return count; */
/* } */
void read_lines(char *path)
{
  FILE *stream;
  ssize_t read;
  char *lnptr = NULL;
  size_t n = 0;
  /* size_t count = 0; */
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  while ((read = getline(&lnptr, &n, stream)) != -1) {
    /* throw away empty lines */
    if (!isempty(lnptr)) {
      assert(("line count too large", number_of_lines < MAX_SRC_LINES));
      src_lines[number_of_lines++] = lnptr;
    }
    lnptr = NULL;
  }
  free(lnptr);
  fclose(stream);
}





#define TOK_MAX_LEN 50		/* max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */
/* char line_toks[MAX_LINE_TOKS][TOK_MAX_LEN];		/\* tokens in 1 line *\/ */



struct token line_toks[MAX_LINE_TOKS];		/* tokens in 1 line */

int tokidx = 0;

/* int tokenize_line(char *line, const char *TOKPATT, int line_num) */
/* { */
/*   regex_t re; */
/*   int errcode;			 */
/*   if ((errcode = regcomp(&re, TOKPATT, REG_EXTENDED))) { /\* compilation failed *\/ */
/*     size_t buff_size = regerror(errcode, &re, NULL, 0); /\* inspect the required buffer size *\/ */
/*     char buff[buff_size+1];	/\* need +1 for the null terminator??? *\/ */
/*     (void)regerror(errcode, &re, buff, buff_size); */
/*     fprintf(stderr, "parse error\n"); */
/*     fprintf(stderr, "regcomp failed with: %s\n", buff); */
/*     exit(errcode); */
/*   } */
/*   regmatch_t match[1];	/\* interesed only in the whole match *\/ */
/*   int offset = 0, count = 0, tokstrlen; */
/*   while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) { /\* match found *\/ */
/*     tokstrlen = match[0].rm_eo - match[0].rm_so; */
/*     struct token t; */
/*     memcpy(t.str, line + offset + match[0].rm_so, tokstrlen); */
/*     t.str[tokstrlen] = '\0'; */
/*     t.idx = tokidx++; */
/*     t.so = offset + match[0].rm_so; */
/*     t.eo = t.so + tokstrlen; */
/*     /\* memcpy(line_toks[count], line + offset +match[0].rm_so, tokstrlen); *\/ */
/*     /\* line_toks[count++][tokstrlen] = '\0'; *\/ */
/*     t.line = line_num; */
/*     line_toks[count++] = t; */
/*     offset += match[0].rm_eo; */
/*   } */
/*   regfree(&re); */
/*   return count; */
/* } */

struct token *tokenize_line2(char *line, int line_num, int *toks_count)
{
  regex_t re;
  int errcode;			
  if ((errcode = regcomp(&re, TOKPATT, REG_EXTENDED))) { /* compilation failed */
    size_t buff_size = regerror(errcode, &re, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &re, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(errcode);
  }
  /* ********************* */
  struct token *toksp = malloc(sizeof(struct token) * MAX_LINE_TOKS);
  /* ************************ */
  struct token *toksp2 = toksp;
  regmatch_t match[1];	/* interesed only in the whole match */
  int offset = 0, tokstrlen;
  while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) { /* match found */
    tokstrlen = match[0].rm_eo - match[0].rm_so;
    struct token t;
    memcpy(t.str, line + offset + match[0].rm_so, tokstrlen);
    t.str[tokstrlen] = '\0';
    t.idx = tokidx++;
    t.so = offset + match[0].rm_so;
    t.eo = t.so + tokstrlen;
    /* memcpy(line_toks[count], line + offset +match[0].rm_so, tokstrlen); */
    /* line_toks[count++][tokstrlen] = '\0'; */
    t.line = line_num;
    *toksp2 = t;
    /* line_toks[count++] = t; */
    (*toks_count)++;
    toksp2++;
    offset += match[0].rm_eo;
  }
  regfree(&re);
  return toksp;
}



void tokenize_src(char *path)
{
  (void)read_lines(path);
  int toks_count = 0;
  for (int i = 0; i < number_of_lines; i++) {
    /* src_toks[i] = tokenize_line2(src_lines[i], i); */
    struct line l;
    l.toks = tokenize_line2(src_lines[i], i, &toks_count);
    l.toks_count = toks_count;
    src_toks[i] = l;
    toks_count = 0;		/* reset */
  }
}

void free_parser(void)
{
  for (int i = 0; i < number_of_lines; i++) {
    free(src_toks[i].toks);
    /* free(src_toks[i]); */
  }
}
