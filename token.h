#ifndef ARV_TOK_H
#define ARV_TOK_H

#include "type.h"

/* ******* reserved keywords, Named strings and characters ******** */
/* differentiate btwn assignment and association like W. Richard Stark pg. 97*/
/* Deprecated (delete) */
#define FUNCALL_KEYWORD "pass"
#define CPACK_KW "cpack"
#define CITH_KW "cith"

#define ARITY_ID ':'                      /* identifier for forms/ops indefinite subexpressions counter  */
#define REST_PARAM_ID '&'              /* identifier for a lambda with a rest-args param (param for an indefinite number of args) */
#define PARAM_WITH_DFLT_ARG_ID '@'              /* identifier for param names with default arg */

#define DEFINEKW "Define"		/* used to define symbols in the global environment */


#define LET_KW "Let"
#define LET_KW_SZ 3

#define LAMBDA_KW "Lambda"
#define LAMBDA_KW_SZ 6

#define PACK_BINDING_PREFIX '&'
#define BINDING_SUFFIX ':'
#define REST_PARAM_PFX '&'      /* rest parameter prefix */
#define MAND_PARAM_SFX ':'        /* mandatory parameter suffix */
#define OPT_PARAM_SFX ":="        /* optional parameter suffix (default arg supplied) */
/* call prefix */
#define CALL_PRFX "Call"
#define CALL_KW "Call"
#define CALL_KW_SZ 4
/* specifying number of args to call */
#define CALL_ARG_PFX '@'
/* specifying number of calls */
#define CALL_RPT_PFX '#'
#define CALL_ULTD_ARG_CHAR '&'
#define TRUEKW "T"
#define FALSEKW "F"
#define CONDKW "Case"
#define CONDIFKW "If"
#define CONDTHENKW "Then"
#define CONDELSEKW "Else"

/* operators */
#define NTHOPKW "Nth"
#define SIZEOPKW "Size"
#define SHOWOPKW "Show"
/* List denotes an empty list, it has max capacity of 0 elements */
#define LIST_OP_KW "List"
#define LIST_OP_KW_SZ 4

#define ADD_OP_KW "Add"
#define ADD_OP_KW_SZ 3
#define MUL_OP_KW "Mul"
#define MUL_OP_KW_SZ 3
#define SUB_OP_KW "Sub"
#define SUB_OP_KW_SZ 3
#define DIV_OP_KW "Div"
#define DIV_OP_KW_SZ 3
#define LFOLD_OP_KW "Lfold"
#define LFOLD_OP_KW_SZ 5

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
    char *string;               /* to be malloced */
    size_t string_size;
    int col_start_idx;			/* start index in line (column start index) */
    int col_end_idx;			/* end index in line (column end index) */
    int line;			/* line number */
    int id;			/* id of this token (tracked globally) */
    size_t comment_couple_tag;
    enum Type type;		/* guessed types at token-generation time */
};

#endif	/* ARV_TOK_H */
