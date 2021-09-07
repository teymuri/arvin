#define _GNU_SOURCE
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
#include <errno.h>
/* #include "parser.h" */

/* naming convention:
   global variables have a leading underscore _foo_bar_baz
*/


int G_srctoks_count = 0;

/* maximum number of lines allowed in one source file */
#define MAXSRCLNS 100
char *G_source_lines[MAXSRCLNS];	/* source lines */
/* number of source lines saved into source_ */
size_t G_source_lines_count = 0;

void null_srclns(void)
{
  for (int i = 0; i < MAXSRCLNS; ++i)
    G_source_lines[i] = NULL;
}

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"
#define MAX_TOKLEN 50		/* bytes max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */

struct token {
  int so;			/* start index in line */
  int eo;			/* end index in line (last char was just the one before this index!) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  char str[MAX_TOKLEN];	/* token's string */
};

struct token2 {
  int so;			/* start index in line */
  int eo;			/* end index in line (last char was just the one before this index!) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  char str[MAX_TOKLEN];	/* token's string */
  struct token2 *nxtok;	/* nächstes token */
};

/* struct line { */
/*   int token_count; */
/*   struct token *tokens; */
/* }; */

/* struct line G_src_toks[MAXSRCLNS]; */

struct token line_toks[MAX_LINE_TOKS];		/* tokens in 1 line */



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


/*
 * reads the source file 'path' and puts non-empty lines into the
 * array G_source_lines. increments the number of lines
 * G_source_lines_count for each line saved.
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
    /* throw away empty lines */
    if (!isempty(lineptr)) {
      /* assert(("line count too large", G_source_lines_count < MAXSRCLNS)); */
      G_source_lines[G_source_lines_count++] = lineptr;
      lineptr = NULL;
    }
  }
  /* free the lineptr variable defined and allocated on this stack */
  /* don't forget to free it's copies in G_source_lines when done with them! */
  free(lineptr);
  fclose(stream);
}

void free_source_lines(void)
{
  for (size_t ln = 0; ln < G_source_lines_count; ++ln)
    free(G_source_lines[ln]);
}



#define MAX_TOKLEN 50		/* max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */
/* char line_toks[MAX_LINE_TOKS][MAX_TOKLEN];		/\* tokens in 1 line *\/ */



struct token line_toks[MAX_LINE_TOKS];		/* tokens in 1 line */
int G_tokid = 0;

struct token *tokenize_line(char *line, int linum)
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
  struct token *tokens = NULL;
  size_t tokens_size = 0;
  int tokscnt = 0;
  while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) { /* a match found */
    /* make room for the new token */
    tokens_size += sizeof(struct token);
    tokens = (struct token *)realloc(tokens, tokens_size); /* tokens must be freed up */
    if (tokens != NULL) {
      tokstrlen = match[0].rm_eo - match[0].rm_so;
      struct token t;
      memcpy(t.str, line + offset + match[0].rm_so, tokstrlen);
      t.str[tokstrlen] = '\0';
      t.id = G_tokid++;
      t.so = offset + match[0].rm_so;
      t.eo = t.so + tokstrlen;
      t.linum = linum;
      /* *(tokens + G_srctoks_count) = t; */
      /* G_srctoks_count++; */
      *(tokens + tokscnt) = t;
      tokscnt++;
      offset += match[0].rm_eo;
    } else {
      fprintf(stderr, "realloc failed while tokenizing line %d", linum);
      /* also free the old tokens memory block (and set return value to NULL) */
      free(tokens);
      tokens = NULL;
      break;
    }
  }
  regfree(&re);
  /* printf(">> %s, ", (tokens)->str); */
  /* printf("\n== %p ==\n", (void *)tokens); */
  return tokens;
}






struct token **tokenize_source(char *path)
{
  (void)read_lines(path);
  /* struct token *linetoks[G_source_lines_count]; */
  struct token **linetoks = malloc(sizeof(struct token *) * (G_source_lines_count + 1));
  size_t ln = 0;
  for (; ln < G_source_lines_count; ln++) {
    if ((linetoks[ln] = tokenize_line(G_source_lines[ln], ln)) != NULL) {
      printf("happy\n");
      free(linetoks[ln]);
    }
  }
  /* printf("%d", ln); */
  /* linetoks[G_source_lines_count] = NULL;		/\* close the array with a null pointer *\/ */
  free(linetoks);
  return linetoks;
}

void free_source_tokens(struct token **linetoks)
{
  while (*linetoks) {
    free(*linetoks);
    (*linetoks)++;
  }
  free(linetoks);
}

/* *********************************************************** */
int main()
{
  /* size_t tokens_size = 0; */
  /* char *s = "let It  Be 2009"; */
  /* struct token *x =tokenize_line(s, 23); */
  /* struct token *p = x; */
  /* printf("%d match in '%s' size %zu\n", G_srctoks_count,s, tokens_size); */
  /* for (int i = 0; i < G_srctoks_count; i++, p++) */
  /*   {printf("%s %d %d sizeof: %zu\n", p->str, p->so, p->id, sizeof p); */
  /*     printf("struct token ptr size: %zu / %zu\n", sizeof(struct token *), sizeof x);} */
  /* free(x); */

  (void)read_lines("/home/okavango/Work/let/etude.let");
  free_source_lines();

  /* struct token **t = tokenize_source("/home/okavango/Work/let/etude.let");   */
  /* free_source_tokens(t); */
    
  exit(EXIT_SUCCESS);
}

/* *********************************************************** */
