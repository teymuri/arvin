#ifndef LET_TOK_H
#define LET_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
#define DEFINE_KW "DFN"		/* used to define symbols in the global environment */
#define LET_KW "LET"
#define FUNCTION_KW "FNC"
#define FUNCALL_KEYWORD "pass"
#define CPACK_KW "cpack"
#define PACK_BINDING_PREFIX '&'
#define BINDING_SUFFIX ':'
#define LTD_CALL_PREFIX "CALL/"
#define CITH_KW "cith"
#define TRUE_KW "T"
#define FALSE_KW "F"
#define CALL_KW "CALL"
#define TILA_NTH_KW "TILA_NTH"
#define TILA_SIZE_KW "TILA_SZ"

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*&_]+)"

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
