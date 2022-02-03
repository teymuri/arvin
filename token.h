#ifndef LET_TOK_H
#define LET_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
#define ASSIGNMENT_KEYWORD "define"		/* used to define symbols in the global environment */
#define ASSOCIATION_KEYWORD "let"
#define LAMBDA_KEYWORD "lambda"
#define FUNCALL_KEYWORD "pass"
#define PACK_BIND_TOK '&'
#define BINDING_TOKEN ':'

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMENT_OPENING "("		/* comment opening token */
#define COMMENT_CLOSING ")"		/* comment closing token */


#define MAX_TOKLEN 50		/* max token length in bytes */

struct Token {
  char str[MAX_TOKLEN];	/* token's string */
  int col_start_idx;			/* start index in line (column start index) */
  int column_end_idx;			/* end index in line (column end index) */
  int line;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comment_index;			/* comment indices: 0 = (, 1 = ) */
  enum Type type;		/* guessed types at token-generation time */
};

#endif	/* LET_TOK_H */
