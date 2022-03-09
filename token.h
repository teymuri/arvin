#ifndef LET_TOK_H
#define LET_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
/* Deprecated (delete) */
#define FUNCALL_KEYWORD "pass"
#define CPACK_KW "cpack"
#define CITH_KW "cith"

#define DEFINEKW "Define"		/* used to define symbols in the global environment */
#define LETKW "Let"
#define LAMBDAKW "Lambda"
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
#define TRUEKW "T"
#define FALSEKW "F"
#define CONDKW "Cond"
#define CONDIFKW "If"
#define CONDTHENKW "Then"
#define CONDELSEKW "Else"

/* operators */
#define NTHOPKW "Nth"
#define SIZEOPKW "Size"
#define SHOWOPKW "Show"
/* List denotes an empty list, it has max capacity of 0 elements */
#define LISTOPKW "List"
#define ADDOPKW "Add"
#define MULOPKW "Mul"
#define SUBOPKW "Sub"
#define DIVOPKW "Div"
#define EXPOPKW "Exp"
#define INCOPKW "Inc"
#define DECOPKW "Dec"
#define LFOLDOPKW "Lfold"

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
