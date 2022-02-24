#ifndef LET_TOK_H
#define LET_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
#define DEFINE_KW "Define"		/* used to define symbols in the global environment */
#define LET_KW "Let"
#define FUNCTION_KW "Lambda"
#define FUNCALL_KEYWORD "pass"
#define CPACK_KW "cpack"
#define PACK_BINDING_PREFIX '&'
#define BINDING_SUFFIX ':'
#define REST_PARAM_PFX '&'      /* rest parameter prefix */
#define MAND_PARAM_SFX ':'        /* mandatory parameter suffix */
#define OPT_PARAM_SFX ":="        /* optional parameter suffix (default arg supplied) */
#define LTD_CALL_PREFIX "Call/"
#define CITH_KW "cith"
#define TRUE_KW "TRUE"
#define FALSE_KW "FALSE"
#define CALL_KW "Call"
#define TILA_NTH_KW "Tila_nth"
#define TILA_SIZE_KW "Tila_size"
#define TILA_SHOW_KW "Tila_show"
#define TILA_LIST_KW "Tila_list"

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*&_]+)"

#define COMMENT_OPENING "("		/* comment opening token */
#define COMMENT_CLOSING ")"		/* comment closing token */


#define MAX_TOKLEN 50		/* max token length in bytes */

struct Token {
    char str[MAX_TOKLEN];	/* token's string */
    int col_start_idx;			/* start index in line (column start index) */
    int col_end_idx;			/* end index in line (column end index) */
    int line;			/* line number */
    int id;			/* id of this token (tracked globally) */
    int comment_index;			/* comment indices: 0 = (, 1 = ) */
    enum Type type;		/* guessed types at token-generation time */
};

#endif	/* LET_TOK_H */
