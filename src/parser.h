#define MAXSRC 20		/* max lines in a src */

char *srclns[MAXSRC];	/* source lines add max line length*/
size_t read_lines(char *);
void free_srclns(size_t);
