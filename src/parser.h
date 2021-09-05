
#define MAX_SRC_LINES 100		/* max lines in the src file */
char *src_lines[MAX_SRC_LINES];	/* source lines add max line length*/

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"
#define TOK_MAX_LEN 50		/* bytes max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */

struct token {
  int so, eo, line, idx;
  char str[TOK_MAX_LEN];
};

struct line {
  int toks_count;
  struct token *toks;
};

struct line src_toks[MAX_SRC_LINES];

struct token line_toks[MAX_LINE_TOKS];		/* tokens in 1 line */

#define MAXSRC 100		/* max lines in a src */
char *srclns[MAXSRC];	/* source lines add max line length*/

int tokenize_line(char *, const char *, int);
struct token *tokenize_line2(char *, int, int *);
void tokenize_src(char *);
void read_lines(char *);
void free_srclns(size_t);
int number_of_lines;
void free_parser(void);
