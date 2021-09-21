

#define MAX_TOKLEN 50		/* bytes max token length */

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int sidx;			/* start index in line */
  int eidx;			/* end index in line */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comidx;			/* comment indices: 0 = (, 1 = ) */
  /* values set at token resolution */
  int ival;
  double fval;
};

struct token *tokenize_source(char *, size_t *);
struct token *remove_comments(struct token *, size_t *, size_t);

