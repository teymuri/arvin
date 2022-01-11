/*
if using glib compile with:

gcc -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -o /tmp/read read.c

*/

#define _GNU_SOURCE

/* #include <string.h> */


#include <stdbool.h>
#include <regex.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "type.h"

/* #include "read.h" */

/* Constructs beginning with an underscore and a capital are internal
   and may be modified or removed any time without notice... */


/* char *stringify_cell_type(enum _Type); */





#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMENT_OPENING "("		/* comment opening token */
#define COMMENT_CLOSING ")"		/* comment closing token */




/* naming convention:
   global variables have 2 leading underscores and a Capital letter
*/

int __Tokid = 1;		/* id 0 is reserved for the toplevel
				   token */

/* int isempty(char *s); */
/* char **read_lines__Hp(char *path, size_t *count); */
/* void free_lines(char **lines, size_t count); */

/* struct token *tokenize_line__Hp */
/* (char *line, size_t *line_toks_count, size_t *all_tokens_count, int linum); */

/* struct token *tokenize_source__Hp(char *path, size_t *all_tokens_count); */

/* struct token *tokenize_lines__Hp */
/* (char **srclns, size_t lines_count, size_t *all_tokens_count); */

/* int is_comment_opening(struct token tok); */
/* int is_comment_closing(struct token tok); */
/* void index_comments(struct token *tokens, size_t all_tokens_count); */
/* struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count, */
/* 				  size_t all_tokens_count); */
/* struct cell *linked_cells__Hp(struct token tokens[], size_t count); */
/* void free_linked_cells(struct cell *c); */







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



char **read_lines__Hp(char *path, size_t *count)
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
      if ((srclns = realloc(srclns, (*count + 1) * sizeof(char *))) != NULL) {
	*(srclns + (*count)++) = lineptr;
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

/* Generates tokens */
struct token *tokenize_line__Hp
(char *line, size_t *line_toks_count, size_t *all_tokens_count, int linum)
{
  regex_t re;
  int errcode;			
  if ((errcode = regcomp(&re, TOKPATT, REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &re, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &re, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(errcode);
  }

  /* For type guessing */
  regex_t reint, refloat, resym;
  if ((errcode = regcomp(&reint, "^[-]*[0-9]+$", REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &reint, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &reint, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
  }
  if ((errcode = regcomp(&refloat, "^[-]*[0-9]*\\.([0-9]*)?$", REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &refloat, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &refloat, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
  }
  if ((errcode = regcomp(&resym, TOKPATT, REG_EXTENDED))) { /* compilation failed (0 = successful compilation) */
    size_t buff_size = regerror(errcode, &resym, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &resym, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(EXIT_FAILURE);
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

      /* guess type */
      if (!regexec(&reint, t.str, 0, NULL, 0)) {
	t.type = INTEGER;
      } else if (!regexec(&refloat, t.str, 0, NULL, 0)) {
	t.type = FLOAT;
      } else if (!regexec(&resym, t.str, 0, NULL, 0)) {
	t.type = SYMBOL;
      } else {
	/* fprintf(stderr, "couldn't guess type of token %s", t.str); */
	/* exit(EXIT_FAILURE); */
	t.type = UNDEFINED;
      }   
      /* t.numtype = numtype(t.str); */
      /* t.isprim = isprim(t.str); */
      t.id = __Tokid++;
      t.column_start_idx = offset + match[0].rm_so;
      t.column_end_idx = t.column_start_idx + tokstrlen;
      t.linum = linum;
      t.comment_index = 0;
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
  regfree(&reint);
  regfree(&refloat);
  regfree(&resym);
  return tokptr;
}


struct token *tokenize_source__Hp(char *path, size_t *all_tokens_count)
{
  size_t lines_count = 0;
  char **lines = read_lines__Hp(path, &lines_count);
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line__Hp */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line__Hp(lines[l], &line_toks_count, all_tokens_count, l);
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

struct token *tokenize_lines__Hp(char **srclns, size_t lines_count,
				  size_t *all_tokens_count)
{
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line__Hp */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line__Hp(srclns[l], &line_toks_count, all_tokens_count, l);
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
  return tokens;
}


int is_comment_opening(struct token tok) {return !strcmp(tok.str, COMMENT_OPENING);}
int is_comment_closing(struct token tok) {return !strcmp(tok.str, COMMENT_CLOSING);}

/* comment index 1 is the start of an outer-most comment block. this
   function is the equivalent of set_commidx_ip(toks) in the let.py
   file. */
void index_comments(struct token *tokens, size_t all_tokens_count)
{
  int idx = 1;
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (is_comment_opening(tokens[i]))
      tokens[i].comment_index = idx++;
    else if (is_comment_closing(tokens[i]))
      tokens[i].comment_index = --idx;
  }
}

struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count,
				  size_t all_tokens_count) /* nct = non-comment token */
{
  index_comments(toks, all_tokens_count);
  struct token *nctoks = NULL;	/* non-comment tokens */
  int isincom = false;		/* are we inside of a comment block? */
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (toks[i].comment_index == 1) {
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





/* 
welche konstrukte generieren neue eigene envs?
* let
* lambda
 */

/* int __Envid = 0; */
/* 
name foo
       34
 */

/* struct env *make_env__Hp(int id, struct env *parenv) */
/* { */
/*   struct env *e = malloc(sizeof(struct env)); */
/*   e->id = id; */
/*   e->hashtable = g_hash_table_new(g_str_hash, g_str_equal); /\* empty hashtable *\/ */
/*   e->symcount = 0; */
/*   e->parenv = parenv; */
/*   return e; */
/* } */



/* struct block __TLBlock = { */
/*   /\* id *\/ */
/*   0, */
/*   /\* cells *\/ */
/*   { */
/*     {				/\* cells[0] *\/ */
/*       /\* car token *\/ */
/*       {.str = TLTOKSTR, */
/*        .column_start_idx = -1, */
/*        .column_end_idx = 100,		/\* ???????????????????????????????????????? *\/ */
/*        .linum = -1, */
/*        .id = 0 */
/*       }, */
/*       /\* cdr cell pointer *\/ */
/*       NULL, */
/*       /\* in block cdr *\/ */
/*       NULL, */
/*       /\* type ??? *\/ */
/*       UNDEFINED, */
/*       NULL,			/\* linker *\/ */
/*       .ival=0,			/\* ival *\/ */
/*       .fval=0.0			/\* fval *\/ */
/*     } */
/*   }, */
  
/*   /\* env (Toplevel Environment) *\/ */
/*   { */
/*     /\* id *\/ */
/*     0, */
/*     /\* hashtable (GHashtable *), to be populated yet *\/ */
/*     NULL, */
/*     /\* parenv *\/ */
/*     NULL, */
/*     /\* symcount *\/ */
/*     0 */
/*   }, */
  
/*   /\* size *\/ */
/*   1, */
/*   /\* child_blocks_count *\/ */
/*   0, */
/*   /\* child_blocks *\/ */
/*   NULL, */
/*   NULL */
/* }; */




/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/

/* bool looks_like_parameter(struct cell *c) */
/* { */
/*   return celltype(c) == SYMBOL && *c->car.str == PARAM_PREFIX; */
/* } */


/* returns a list of linked cells made of tokens */
struct cell *linked_cells__Hp(struct token tokens[], size_t count)
{
  struct cell *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct cell *c = malloc(sizeof (struct cell));
    if (i == 0) root = c;
    
    /* guess_token_type(tokens+i);	/\* pass the pointer to the token *\/ */
    
    c->car = tokens[i];
    if (i > 0)
      prev->cdr = c;
    if (i == count-1)
      c->cdr = NULL;
    set_cell_value(c);
    set_cell_type(c);
    prev = c;
  }
  return root;
}

void free_linked_cells(struct cell *c)
{
  struct cell *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->cdr;
    free(tmp);
  }
}
