/* read.c is only concerned with generating tokens from the source */


/*
  if using glib compile with:

  gcc -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -o /tmp/read read.c

*/

#define _GNU_SOURCE

#include <string.h>
#include <stdbool.h>
#include <regex.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include "type.h"
#include "token.h"

/* #include "read.h" */

/* Constructs beginning with an underscore and a capital are internal
   and may be modified or removed any time without notice... */


/* char *stringify_cell_type(enum Type); */








/* naming convention:
   global variables have 2 leading underscores and a Capital letter
*/

int Token_id = 1;		/* id 0 is reserved for the toplevel
				   token */






/* checks if the string s consists only of blanks and/or newline */
int isempty(char *s) {
    while (*s) {
        /* if char is something other than a blank or a newline, the string
           is regarded as non-empty. */
        if (!isblank(*s) && *s != '\n')
            return 0;
        s++;
    }
    return 1;
}



char **
read_lines(char *path, size_t *lines_count)
{
    FILE *stream;
    stream = fopen(path, "r");
    if (!stream) {
        fprintf(stderr, "Arvin can't open the file '%s'\n", path);
        exit(EXIT_FAILURE);
    }
    char *lineptr = NULL;
    size_t n = 0;
    char **lines = NULL;
    while ((getline(&lineptr, &n, stream) != -1)) {
        if (!isempty(lineptr)) {
            if ((lines = realloc(lines, (*lines_count + 1) * sizeof (char *)))) {
                *(lines + *lines_count) = lineptr;
                (*lines_count)++;
                lineptr = NULL;
            } else
                exit(EXIT_FAILURE);
        }
    }
    free(lineptr);
    fclose(stream);
    return lines;
}

void
free_lines(char **lines, size_t lines_count)
{
    for (size_t i = 0; i < lines_count; i++)
        free(lines[i]);
    free(lines);
    /* char **base = lines; */
    /* while (lines_count--) free(*lines++); */
    /* free(base); */
}

void
free_line_toks(struct Token **line_toks,
               size_t line_toks_count)
{
    for (size_t i = 0; i < line_toks_count; i++)
        free(line_toks[i]);
    free(line_toks);
}

/* Generates tokens */
struct Token *
tokenize_line__Hp(char *lnstr,
                  size_t *line_toks_count,
                  size_t *all_tokens_count,
                  int ln)
/* lnstr = line string content, ln = line number*/
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
    struct Token *line_tokens = NULL;
    /* overall size of memory allocated for tokens of the line sofar */
    size_t memsize = 0;
    /* int tokscnt = 0; */
    while (!regexec(&re, lnstr + offset, 1, match, REG_NOTBOL)) { /* a match found */
        /* make room for the new token */
        memsize += sizeof(struct Token);
        if ((line_tokens = realloc(line_tokens, memsize))) {
            tokstrlen = match[0].rm_eo - match[0].rm_so;
            struct Token t;
            memcpy(t.str, lnstr + offset + match[0].rm_so, tokstrlen);
            t.str[tokstrlen] = '\0';

            /* guess type */
            if (!regexec(&reint, t.str, 0, NULL, 0)) {
                t.type = INT;
            } else if (!regexec(&refloat, t.str, 0, NULL, 0)) {
                t.type = FLOAT;
            } else if (!regexec(&resym, t.str, 0, NULL, 0)) {
                t.type = NAME;
            } else {
                /* fprintf(stderr, "couldn't guess type of token %s", t.str); */
                /* exit(EXIT_FAILURE); */
                t.type = UNDEFINED;
            }   
            /* t.numtype = numtype(t.str); */
            /* t.isprim = isprim(t.str); */
            t.id = Token_id++;
            t.col_start_idx = offset + match[0].rm_so;
            t.col_end_idx = t.col_start_idx + tokstrlen;
            t.line = ln;
            t.comment_index = 0;
            *(line_tokens + *line_toks_count) = t;
            (*all_tokens_count)++;
            (*line_toks_count)++;
            offset += match[0].rm_eo;
        } else {
            fprintf(stderr, "realloc failed while tokenizing line %d at token %s", ln, "TOKEN????");
            /* just break out of executaion if haven't enough memory for the
               next token. leave the freeing & cleanup over for the os! */
            exit(EXIT_FAILURE);
        }
    }
    regfree(&re);
    regfree(&reint);
    regfree(&refloat);
    regfree(&resym);
    return line_tokens;
}

void
free_token(struct Token *token)
{
    free(token->string);
    free(token);
}

/* void */
/* free_line_tokens(struct Token **line_tokens, */
/*                  size_t line_tokens_count) */
/* { */
/*     for (size_t i = 0; i < line_tokens_count; i++) */
/*         free_token(line_tokens[i]); */
/*     free(line_tokens); */
/* } */

struct Token **
tokenize_line(char *lnstr,
              size_t *line_toks_count,
              size_t *all_tokens_count,
              int ln)
/* lnstr = line string content, ln = line number*/
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
    int offset = 0;
    int token_size;
    struct Token **line_tokens = NULL;
    /* overall size of memory allocated for tokens of the line sofar */
    /* size_t memsize = 0; */
    /* int tokscnt = 0; */
    while (!regexec(&re, lnstr + offset, 1, match, REG_NOTBOL)) { /* a match found */
        /* make room for the new token */
        /* memsize += sizeof(struct Token); */
        if ((line_tokens = realloc(line_tokens,
                                   (*line_toks_count + 1) * sizeof (struct Token *)))) {
            struct Token *token = malloc(sizeof (struct Token));
            token_size = match[0].rm_eo - match[0].rm_so; /* remove this!!! look below */
            token->string_size = match[0].rm_eo - match[0].rm_so;
            token->string = malloc(token->string_size + 1); /* for null terminator? */
            /* struct Token t; */
            /* memcpy(t.str, lnstr + offset + match[0].rm_so, tokstrlen); */
            /* t.str[tokstrlen] = '\0'; */
            memcpy(token->string, lnstr + offset + match[0].rm_so, token_size);
            token->string[token_size] = '\0';
            /* guess type */
            if (!regexec(&reint, token->string, 0, NULL, 0)) {
                /* t.type = INT; */
                token->type = INT;
            } else if (!regexec(&refloat, token->string, 0, NULL, 0)) {
                token->type = FLOAT;
            } else if (!regexec(&resym, token->string, 0, NULL, 0)) {
                token->type = NAME;
            } else {
                /* fprintf(stderr, "couldn't guess type of token %s", t.str); */
                /* exit(EXIT_FAILURE); */
                token->type = UNDEFINED;
            }   
            /* t.numtype = numtype(t.str); */
            /* t.isprim = isprim(t.str); */
            token->id = Token_id++;
            token->col_start_idx = offset + match[0].rm_so;
            token->col_end_idx = token->col_start_idx + token->string_size;
            token->line = ln;
            token->comment_index = 0;
            *(line_tokens + *line_toks_count) = token;
            (*all_tokens_count)++;
            (*line_toks_count)++;
            offset += match[0].rm_eo;
        } else {
            fprintf(stderr, "realloc failed while tokenizing line '%d' at token '%s'", ln, "???");
            /* just break out of executaion if haven't enough memory for the
               next token. leave the freeing & cleanup over for the os! */
            exit(EXIT_FAILURE);
        }
    }
    regfree(&re);
    regfree(&reint);
    regfree(&refloat);
    regfree(&resym);
    return line_tokens;
}

void
tokenize_line2(char *lnstr,
               int ln,
               struct Token **source_tokens,
               size_t *source_tokens_count
               /* char *source_path */
    )
/* lnstr = line string content, ln = line number*/
{
    /* size_t lines_count = 0; */
    /* char **lines = read_lines(source_path, &lines_count); */

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
    int offset = 0;
    int token_size;
    
    /* struct Token **line_tokens = NULL; */
    
    /* overall size of memory allocated for tokens of the line sofar */
    /* size_t memsize = 0; */
    /* int tokscnt = 0; */
    /* struct Token *source_tokens = NULL; */
    
    while (!regexec(&re, lnstr + offset, 1, match, REG_NOTBOL)) { /* a match found */
        /* make room for the new token */
        /* memsize += sizeof(struct Token); */

        
        struct Token *token_ptr = malloc(sizeof (struct Token));
        token_size = match[0].rm_eo - match[0].rm_so; /* remove this!!! look below */
        token_ptr->string_size = match[0].rm_eo - match[0].rm_so;
        token_ptr->string = malloc(token_ptr->string_size + 1); /* for null terminator? */
        /* struct Token t; */
        /* memcpy(t.str, lnstr + offset + match[0].rm_so, tokstrlen); */
        /* t.str[tokstrlen] = '\0'; */
        memcpy(token_ptr->string, lnstr + offset + match[0].rm_so, token_size);
        token_ptr->string[token_size] = '\0';
        /* guess type */
        if (!regexec(&reint, token_ptr->string, 0, NULL, 0)) {
            /* t.type = INT; */
            token_ptr->type = INT;
        } else if (!regexec(&refloat, token_ptr->string, 0, NULL, 0)) {
            token_ptr->type = FLOAT;
        } else if (!regexec(&resym, token_ptr->string, 0, NULL, 0)) {
            token_ptr->type = NAME;
        } else {
            /* fprintf(stderr, "couldn't guess type of token %s", t.str); */
            /* exit(EXIT_FAILURE); */
            token_ptr->type = UNDEFINED;
        }   
        /* t.numtype = numtype(t.str); */
        /* t.isprim = isprim(t.str); */
        token_ptr->id = Token_id++;
        token_ptr->col_start_idx = offset + match[0].rm_so;
        token_ptr->col_end_idx = token_ptr->col_start_idx + token_ptr->string_size;
        token_ptr->line = ln;
        token_ptr->comment_index = 0;

        if ((*source_tokens = realloc(*source_tokens, (*source_tokens_count + 1) * sizeof (struct Token)))) {
            /* *(source_tokens[*source_tokens_count]) = token_ptr; */
            source_tokens[*source_tokens_count] = token_ptr;
            /* source_tokens + *source_tokens_count = token_ptr; */
            (*source_tokens_count)++;
        } else {
            fprintf(stderr, "tokenize_line2 failed while reallocing\n");
            exit(EXIT_FAILURE);
        }
        
        /* *(line_tokens + *line_toks_count) = token; */
        /* (*all_tokens_count)++; */
        /* (*line_toks_count)++; */
        offset += match[0].rm_eo;
        

        
        /* if ((line_tokens = realloc(line_tokens, */
        /*                            (*line_toks_count + 1) * sizeof (struct Token *)))) { */
        /*     struct Token *token = malloc(sizeof (struct Token)); */
        /*     token_size = match[0].rm_eo - match[0].rm_so; /\* remove this!!! look below *\/ */
        /*     token->string_size = match[0].rm_eo - match[0].rm_so; */
        /*     token->string = malloc(token->string_size + 1); /\* for null terminator? *\/ */
        /*     /\* struct Token t; *\/ */
        /*     /\* memcpy(t.str, lnstr + offset + match[0].rm_so, tokstrlen); *\/ */
        /*     /\* t.str[tokstrlen] = '\0'; *\/ */
        /*     memcpy(token->string, lnstr + offset + match[0].rm_so, token_size); */
        /*     token->string[token_size] = '\0'; */
        /*     /\* guess type *\/ */
        /*     if (!regexec(&reint, token->string, 0, NULL, 0)) { */
        /*         /\* t.type = INT; *\/ */
        /*         token->type = INT; */
        /*     } else if (!regexec(&refloat, token->string, 0, NULL, 0)) { */
        /*         token->type = FLOAT; */
        /*     } else if (!regexec(&resym, token->string, 0, NULL, 0)) { */
        /*         token->type = NAME; */
        /*     } else { */
        /*         /\* fprintf(stderr, "couldn't guess type of token %s", t.str); *\/ */
        /*         /\* exit(EXIT_FAILURE); *\/ */
        /*         token->type = UNDEFINED; */
        /*     }    */
        /*     /\* t.numtype = numtype(t.str); *\/ */
        /*     /\* t.isprim = isprim(t.str); *\/ */
        /*     token->id = Token_id++; */
        /*     token->col_start_idx = offset + match[0].rm_so; */
        /*     token->col_end_idx = token->col_start_idx + token->string_size; */
        /*     token->line = ln; */
        /*     token->comment_index = 0; */
        /*     *(line_tokens + *line_toks_count) = token; */
        /*     (*all_tokens_count)++; */
        /*     (*line_toks_count)++; */
        /*     offset += match[0].rm_eo; */
        /* } else { */
        /*     fprintf(stderr, "realloc failed while tokenizing line '%d' at token '%s'", ln, "???"); */
        /*     /\* just break out of executaion if haven't enough memory for the */
        /*        next token. leave the freeing & cleanup over for the os! *\/ */
        /*     exit(EXIT_FAILURE); */
        /* } */

        
    }
    regfree(&re);
    regfree(&reint);
    regfree(&refloat);
    regfree(&resym);
    /* return source_tokens; */
}



struct Token **
tokenize_source(char *path,
                size_t *source_tokens_count)
{
    size_t lines_count = 0;
    char **lines = read_lines(path, &lines_count);
    /* source_tokens are all the tokens found in a script, including
     * comments.*/
    struct Token **source_tokens = NULL;
    /* struct Token *line_tokens = NULL; */
    struct Token **line_tokens = NULL;
    size_t line_toks_count, global_toks_count_cpy;
    
    for (size_t i = 0; i < lines_count; i++) {
        /* struct Token **line_tokens = NULL; */
        line_toks_count = 0;
        /* take a snapshot of the number of source tokens sofar, before
           it's changed by tokenize_line__Hp */
        global_toks_count_cpy = *source_tokens_count;
        /* line_tokens = tokenize_line__Hp(lines[i], &line_toks_count, source_tokens_count, i); */
        line_tokens = tokenize_line(lines[i], &line_toks_count, source_tokens_count, i);
        if ((source_tokens = realloc(source_tokens, *source_tokens_count * sizeof (struct Token)))) {
            for (size_t i = 0; i < line_toks_count; i++) {
                *(source_tokens + i + global_toks_count_cpy) = line_tokens[i];
            }
        } else {
            exit(EXIT_FAILURE);
        }
        free(line_tokens);
        line_tokens = NULL;
    }
    free_lines(lines, lines_count);
    return source_tokens;
}

struct Token *
tokenize_source2(char *path,
                 size_t *source_tokens_count)
{
    size_t lines_count = 0;
    char **lines = read_lines(path, &lines_count);
    /* source_tokens are all the tokens found in a script, including
     * comments.*/
    struct Token *source_tokens = NULL;
    /* struct Token **source_tokens = malloc(sizeof(struct Token)); */
    /* struct Token *line_tokens = NULL; */
    /* struct Token **line_tokens = NULL; */
    /* size_t line_toks_count, global_toks_count_cpy; */

    for (size_t i = 0; i < lines_count; i++) {
        tokenize_line2(lines[i],
                       i,
                       &source_tokens,
                       source_tokens_count);
    }
    
    /* for (size_t i = 0; i < lines_count; i++) { */
    /*     /\* struct Token **line_tokens = NULL; *\/ */
    /*     line_toks_count = 0; */
    /*     /\* take a snapshot of the number of source tokens sofar, before */
    /*        it's changed by tokenize_line__Hp *\/ */
    /*     global_toks_count_cpy = *source_tokens_count; */
    /*     /\* line_tokens = tokenize_line__Hp(lines[i], &line_toks_count, source_tokens_count, i); *\/ */
    /*     line_tokens = tokenize_line(lines[i], &line_toks_count, source_tokens_count, i); */
    /*     if ((source_tokens = realloc(source_tokens, *source_tokens_count * sizeof (struct Token)))) { */
    /*         for (size_t i = 0; i < line_toks_count; i++) { */
    /*             *(source_tokens + i + global_toks_count_cpy) = line_tokens[i]; */
    /*         } */
    /*     } else { */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /*     free(line_tokens); */
    /*     line_tokens = NULL; */
    /* } */
    
    free_lines(lines, lines_count);
    return source_tokens;
}


struct Token *tokenize_lines__Hp(char **lines, size_t lines_count,
                                 size_t *all_tokens_count)
{
    struct Token *tokens = NULL;
    struct Token *lntoks = NULL;
    size_t line_toks_count, global_toks_count_cpy;
    for (size_t l = 0; l < lines_count; l++) {
        line_toks_count = 0;
        /* take a snapshot of the number of source tokens sofar, before
           it's changed by tokenize_line__Hp */
        global_toks_count_cpy = *all_tokens_count;
        lntoks = tokenize_line__Hp(lines[l], &line_toks_count, all_tokens_count, l);
        if ((tokens = realloc(tokens, *all_tokens_count * sizeof(struct Token))) != NULL) {
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


int
is_comment_opening(struct Token *tok)
{
    return !strcmp(tok->string, COMMENT_OPENING);
}

int
is_comment_closing(struct Token *tok)
{
    return !strcmp(tok->string, COMMENT_CLOSING);
}

/* comment index 1 is the start of an outer-most comment block. this
   function is the equivalent of set_commidx_ip(toks) in the let.py
   file. */
static void
index_comments(struct Token *tokens, size_t source_tokens_count)
{
    int idx = 1;
    for (size_t i = 0; i < source_tokens_count; i++) {
        if (is_comment_opening(tokens+i))
            (tokens+i)->comment_index = idx++;
        else if (is_comment_closing(tokens+i))
            (tokens+i)->comment_index = --idx;
    }
}

struct Token **
remove_comments(struct Token *source_tokens,
                size_t *code_tokens_count,
                size_t source_tokens_count) /* nct = non-comment token */
{
    index_comments(source_tokens, source_tokens_count);
    struct Token **code_tokens = NULL;	/* non-comment tokens */
    int isincom = false;		/* are we inside of a comment block? */
    for (size_t i = 0; i < source_tokens_count; i++) {
        if ((source_tokens+i)->comment_index == 1) {
            if (isincom) isincom = false;
            else isincom = true;
        } else if (!isincom) {
            /* not in a comment block, allocate space for the new non-comment token */
            /* (*code_tokens_count)++; */
            if ((code_tokens = realloc(code_tokens, (*code_tokens_count + 1) * sizeof(struct Token)))) {
                *(code_tokens + *code_tokens_count) = (source_tokens+i);
                (*code_tokens_count)++;
            } 
            else
                exit(EXIT_FAILURE);
        }
    }
    free(source_tokens);
    return code_tokens;
}
