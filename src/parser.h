
#define TOK_MAX_LEN 50		/* bytes max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */

struct token {
  int so, eo, line, idx;
  char str[TOK_MAX_LEN];
};

struct token line_toks[MAX_LINE_TOKS];		/* tokens in 1 line */

#define MAXSRC 100		/* max lines in a src */
char *srclns[MAXSRC];	/* source lines add max line length*/

int tokenize_line(char *, const char *, int);
size_t read_lines(char *);
void free_srclns(size_t);
