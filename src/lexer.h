#ifndef __LETLEXER_H
#define __LETLEXER_H

enum __Type {
  NUMBER, INTEGER, FLOAT,
  LAMBDA,
  UNDEFINED
};

char *stringize_type(enum __Type);

#define MAX_TOKLEN 50		/* bytes max token length */
#define TLTOKSTR "__TLTOKSTR__"

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int colsidx;			/* start index in line (column start index) */
  int coleidx;			/* end index in line (column end index) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comidx;			/* comment indices: 0 = (, 1 = ) */
  enum __Type type;
  int ival;
  double fval;
};


struct token *tokenize_source__Hp(char *, size_t *);
struct token *tokenize_srclns__Hp(char **, size_t, size_t *);
struct token *remove_comments__Hp(struct token *, size_t *, size_t);

#endif
