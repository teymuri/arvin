
#define TOK_MAX_LEN 50		/* bytes max token length */
#define MAX_LINE_TOKS 10		/* max number of tokens in 1 line */
char line_toks[MAX_LINE_TOKS][TOK_MAX_LEN];		/* tokens in 1 line */

#define MAXSRC 100		/* max lines in a src */
char *srclns[MAXSRC];	/* source lines add max line length*/

int tokenize_line(char *, const char *);
size_t read_lines(char *);
void free_srclns(size_t);
