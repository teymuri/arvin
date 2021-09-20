#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
#include "prims.h"
/* #include <errno.h> */
/* #include "parser.h" */

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"
#define MAX_TOKLEN 50		/* bytes max token length */
#define COMMOP "("		/* comment opening token */
#define COMMCL ")"		/* comment closing token */

/* naming convention:
   global variables have 2 leading underscores and a Capital letter
*/

int __Tokid = 1;		/* id 0 is reserved for the toplevel
				   token */




struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int sidx;			/* start index in line */
  int eidx;			/* end index in line */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comidx;			/* comment indices: 0 = (, 1 = ) */
  bool isprim;
};






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



char **read_lines(char *path, size_t *count)
{
  FILE *stream;
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  char *lineptr = NULL;
  size_t n = 0;
  char **srclns = NULL;
  while ((getline(&lineptr, &n, stream) != -1)) {
    if (!isempty(lineptr)) {
      /* increment *count first, otherwise realloc will be called with size 0 :-O */
      if ((srclns = realloc(srclns, ++*count * sizeof(char *))) != NULL) {
	*(srclns + *count - 1) = lineptr;
	lineptr = NULL;
      } else exit(EXIT_FAILURE);
    }
  }
  free(lineptr);
  fclose(stream);
  return srclns;
}

void free_lines(char **lines, size_t count)
{
  char **base = lines;
  while (count--) free(*lines++);
  free(base);
}




struct token *tokenize_line(char *line, size_t *line_toks_count, size_t *all_tokens_count, int linum)
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
  regmatch_t match[1];	/* interesed only in the whole match */
  int offset = 0, tokstrlen;
  struct token *tokptr = NULL;
  /* overall size of memory allocated for tokens of the line sofar */
  size_t memsize = 0;
  /* int tokscnt = 0; */
  while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) { /* a match found */
    /* make room for the new token */
    memsize += sizeof(struct token);
    if ((tokptr = realloc(tokptr, memsize)) != NULL) { /* new memory allocated successfully */
      tokstrlen = match[0].rm_eo - match[0].rm_so;
      struct token t;
      memcpy(t.str, line + offset + match[0].rm_so, tokstrlen);
      t.str[tokstrlen] = '\0';
      t.isprim = isprim(t.str);
      t.id = __Tokid++;
      t.sidx = offset + match[0].rm_so;
      t.eidx = t.sidx + tokstrlen;
      t.linum = linum;
      t.comidx = 0;
      *(tokptr + *line_toks_count) = t;
      (*all_tokens_count)++;
      (*line_toks_count)++;
      offset += match[0].rm_eo;
    } else {
      fprintf(stderr, "realloc failed while tokenizing line %d at token %s", linum, "TOKEN????");
      /* just break out of executaion if haven't enough memory for the
	 next token. leave the freeing & cleanup over for the os! */
      exit(EXIT_FAILURE);
    }
  }
  regfree(&re);  
  return tokptr;
}



struct token *tokenize_source(char *path, size_t *all_tokens_count)
{
  size_t lines_count = 0;
  char **lines = read_lines(path, &lines_count);
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line(lines[l], &line_toks_count, all_tokens_count, l);
    if ((tokens = realloc(tokens, *all_tokens_count * sizeof(struct token))) != NULL) {
      for (size_t i = 0; i < line_toks_count; i++) {
	*(tokens + i + global_toks_count_cpy) = lntoks[i];
      }
    } else {
      exit(EXIT_FAILURE);
    }
    free(lntoks);
    lntoks=NULL;
  }
  free_lines(lines, lines_count);
  return tokens;
}

int iscom_open(struct token tok)
{
  return !strcmp(tok.str, COMMOP);
}
int iscom_close(struct token tok)
{
  return !strcmp(tok.str, COMMCL);
}

/* comment index 1 is the start of an outer-most comment block. this
   function is the equivalent of set_commidx_ip(toks) in the let.py
   file. */
void index_comments(struct token *tokens, size_t all_tokens_count)
{
  int idx = 1;
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (iscom_open(tokens[i]))
      tokens[i].comidx = idx++;
    else if (iscom_close(tokens[i]))
      tokens[i].comidx = --idx;
  }
}

struct token *remove_comments(struct token *toks, size_t *nctok_count, size_t all_tokens_count) /* nct = non-comment token */
{
  index_comments(toks, all_tokens_count);
  struct token *nctoks = NULL;	/* non-comment tokens */
  int isincom = false;		/* are we inside of a comment block? */
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (toks[i].comidx == 1) {
      if (isincom) isincom = false;
      else isincom = true;
    } else if (!isincom) {
      /* not in a comment block, allocate space for the new non-comment token */
      /* (*nctok_count)++; */
      if ((nctoks = realloc(nctoks, ++(*nctok_count) * sizeof(struct token))) != NULL)
	/* the index for the new token is one less than the current number of non-comment tokens */
	*(nctoks + *nctok_count - 1) = toks[i];
      else
	exit(EXIT_FAILURE);
    }
  }
  free(toks);
  return nctoks;
}

/* *********************************************************** */
int main()
{
  /* size_t n = 0; */
  /* char **lns = read_lines("/home/amir/a.let", &n); */
  /* printf("%zu\n", n); */
  /* free_lines2(lns, n); */
  size_t all_tokens_count = 0;
  struct token *toks = tokenize_source("/home/amir/a.let", &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments(toks, &nctok_count, all_tokens_count);
  for (size_t i = 0; i<nctok_count;i++) {
    printf("%zu- %s %d\n", i, nct[i].str, nct[i].isprim);
  }
  free(nct);
    
  exit(EXIT_SUCCESS);
}

/* *********************************************************** */
