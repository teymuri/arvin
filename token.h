#ifndef LET_TOK_H
#define LET_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
#define ASSIGNMENT_KEYWORD "define"		/* used to define symbols in the global environment */
#define ASSOCIATION_KEYWORD "let"
#define LAMBDA_KW "lambda"
/* #define PARAM_PREFIX '.' */

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMENT_OPENING "("		/* comment opening token */
#define COMMENT_CLOSING ")"		/* comment closing token */


#define MAX_TOKLEN 50		/* max token length in bytes */

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int column_start_idx;			/* start index in line (column start index) */
  int column_end_idx;			/* end index in line (column end index) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comment_index;			/* comment indices: 0 = (, 1 = ) */
  enum _Type type;		/* guessed types at token-generation time */
};

#endif	/* LET_TOK_H */
