#ifndef LET_TOK_H
#define LET_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
/* Deprecated (delete) */
#define FUNCALL_KEYWORD "pass"
#define CPACK_KW "cpack"
#define CITH_KW "cith"

#define DEFINE_KW "Define"		/* used to define symbols in the global environment */
#define LET_KW "Let"
#define FUNCTION_KW "Lambda"
#define PACK_BINDING_PREFIX '&'
#define BINDING_SUFFIX ':'
#define REST_PARAM_PFX '&'      /* rest parameter prefix */
#define MAND_PARAM_SFX ':'        /* mandatory parameter suffix */
#define OPT_PARAM_SFX ":="        /* optional parameter suffix (default arg supplied) */
/* call prefix */
#define CALL_PRFX "Call"
/* specifying number of args to call */
#define CALL_ARG_PFX '@'
/* specifying number of calls */
#define CALL_RPT_PFX '#'
#define CALL_ULTD_ARG_CHAR '&'
#define TRUE_KW "True"
#define FALSE_KW "False"
/* #define CALL_KW "Call" */
#define TILA_NTH_KW "Nth"
#define TILA_SIZE_KW "Size"
#define TILA_SHOW_KW "Show"
/* List denotes an empty list, it has max capacity of 0 elements */
#define TILA_LIST_KW "List"
#define COND_KW "Cond"
#define COND_IF_KW "If"
#define COND_THEN_KW "Then"
#define COND_ELSE_KW "Else"
#define ADDOP "Add"
#define MULOP "Mul"
#define SUBOP "Sub"
#define DIVOP "Div"
#define EXPOP "Exp"
#define INCOP "Inc"
#define DECOP "Dec"
#define TILA_LFOLD "Lfold"
/*  */
#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*&_@#]+)"

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
