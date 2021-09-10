#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
/* #include <errno.h> */
/* #include "parser.h" */

/* naming convention:
   global variables have a leading underscore _foo_bar_baz
*/


size_t G_source_tokens_count = 0;

/* maximum number of lines allowed in one source file */
#define MAXSRCLNS 100
/* G_source_lines will contain copies of pointers to memory blocks
   allocated by getline, so must be freed later! */
char *G_source_lines[MAXSRCLNS];
/* number of source lines saved into source_ */
size_t G_source_lines_count = 0;


#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"
#define MAX_TOKLEN 50		/* bytes max token length */
#define MAX_LINE_TOKS 10		/* max number of tok in 1 line */

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int so;			/* start index in line */
  int eo;			/* end index in line (last char was just the one before this index!) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
};


struct token line_toks[MAX_LINE_TOKS];		/* tok in 1 line */



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



/*  reads the source file 'path' and puts non-empty lines into the
    array G_source_lines. increments the number of lines
    G_source_lines_count for each line saved.
*/
void read_lines(char *path)
{
  FILE *stream;
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  char *lineptr = NULL;
  size_t n = 0;
  while ((getline(&lineptr, &n, stream) != -1)) {
    /* throw away empty lines. i just resue lineptr on next getline if an
       empty line was put into it by getline. */
    if (!isempty(lineptr)) {
      /* assert(("line count too large", G_source_lines_count < MAXSRCLNS)); */
      G_source_lines[G_source_lines_count++] = lineptr;
      lineptr = NULL;		/* force getline to calc size of
				   needed memory for the next line
				   himself! */
    }
  }
  /* free the lineptr variable defined and allocated on this stack */
  /* don't forget to free it's copies in G_source_lines when done with them! */
  free(lineptr);
  fclose(stream);
}

/* frees copies of line pointers to memory blocks allocated by getline
   in read_lines. this should be done after having processed all
   lineptrs in G_source_lines (i.e. at the end of tokenize_source??). */
void free_lines(void)
{
  for (size_t ln = 0; ln < G_source_lines_count; ++ln)
    free(G_source_lines[ln]);
}



#define MAX_TOKLEN 50		/* max token length */
#define MAX_LINE_TOKS 10		/* max number of tok in 1 line */
/* char line_toks[MAX_LINE_TOKS][MAX_TOKLEN];		/\* tok in 1 line *\/ */



struct token line_toks[MAX_LINE_TOKS];		/* tok in 1 line */
int G_tokid = 0;

struct token *tokenize_line(char *line, size_t *lntoks_count, size_t *srctoks_count, int linum)
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
      t.id = G_tokid++;
      t.so = offset + match[0].rm_so;
      t.eo = t.so + tokstrlen;
      t.linum = linum;
      *(tokptr + *lntoks_count) = t;
      (*srctoks_count)++;
      (*lntoks_count)++;
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



struct token *tokenize_source(char *path)
{
  read_lines(path);
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t lntoks_count, srctoks;
  for (size_t l = 0; l<G_source_lines_count;l++) {
    lntoks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line */
    srctoks = G_source_tokens_count;
    lntoks = tokenize_line(G_source_lines[l], &lntoks_count, &G_source_tokens_count, l);
    if ((tokens = realloc(tokens, sizeof(struct token) * G_source_tokens_count)) != NULL) {
      for (size_t i = 0; i < lntoks_count; i++) {
	*(tokens + i + srctoks) = lntoks[i];
      }
    } else {
      exit(EXIT_FAILURE);
    }
    free(lntoks);
    lntoks=NULL;
  }
  free_lines();
  return tokens;
}



/* *********************************************************** */
int main()
{
  
  struct token *t = tokenize_source("/home/okavango/Work/let/etude.let");
  /* printf("%zu\n", G_source_tokens_count); */
  for (size_t i = 0; i<G_source_tokens_count;i++) {
    printf("%zu- %s %d %d %d\n", i, t[i].str, t[i].so, t[i].eo, t[i].linum);
  }
  /* free_lines(); */
  free(t);
    
  exit(EXIT_SUCCESS);
}

/* *********************************************************** */
