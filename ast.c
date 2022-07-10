#include <stdlib.h>		/* EXIT_FAILURE */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "type.h"
#include "token.h"
#include "unit.h"
#include "print.h"

#define LEAST_COL_START_IDX -2
#define ASSIGN_MAXCAP 2
#define BIND_MAXCAP 1

bool is_enclosed_in3(GList *ulink1, GList *ulink2)
{
    if (ulink2 != NULL) {
        struct Unit *u1 = (struct Unit *)ulink1->data, *u2 = (struct Unit *)ulink2->data;
        return u1->token->col_start_idx > u2->token->col_start_idx &&
            u1->token->line >= u2->token->line;
    } else {
        return false;
    }
}

bool is_enclosed_in4(struct Unit *a1, struct Unit *a2)
{
    return a1->token->col_start_idx > a2->token->col_start_idx &&
        a1->token->line >= a2->token->line;
}

bool
is_enclosed_in5(struct Unit *a1, struct Unit *a2)
{
    if (a1->token->line == a2->token->line) {
        return a1->token->col_start_idx > a2->token->col_start_idx;
    } else if (a1->token->line > a2->token->line) {
        return true;
    } else return false;
}

int bottom_line_number3(GList *list)
{
    int line = -1;
    while (list) {
        if (((struct Unit *)list->data)->token->line > line)
            line = ((struct Unit *)list->data)->token->line;
        list = list->next;
    }
    return line;
}

GList *enclosures3(GList *ulink, GList *list) {
    GList *sll = NULL;
    for (;
         list && ((struct Unit *)list->data)->uuid < ((struct Unit *)ulink->data)->uuid;
         list = list->next) {
        if (!((struct Unit *)list->data)->is_atomic && is_enclosed_in3(ulink, list))
            sll = g_list_append(sll, list->data);
    }
    return sll;
}

/* remove from encs */
GList *bottom_enclosures3(GList *encs) {
    int bot_linum = bottom_line_number3(encs);
    GList *out = NULL;
    while (encs) {
        if (((struct Unit *)encs->data)->token->line == bot_linum)
            out = g_list_append(out, encs->data);
        encs = encs->next;
    }
    return out;
}
GList *rightmost_enclosure(GList *botencs) {
    int col_start_idx = LEAST_COL_START_IDX;
    GList *rmost = NULL;
    while (botencs) {
        if (((struct Unit *)botencs->data)->token->col_start_idx > col_start_idx) {
            col_start_idx = ((struct Unit *)botencs->data)->token->col_start_idx;
            rmost = botencs;
        }
        botencs = botencs->next;
    }
    return rmost;
}

GList *find_enclosure_link(GList *unit_link, GList *tl_ulink) {
    GList *all = enclosures3(unit_link, tl_ulink);
    GList *undermosts = bottom_enclosures3(all);
    return rightmost_enclosure(undermosts);
}


bool
is_lambda4(struct Unit *u)
{
    return !strcmp(u->token->string, LAMBDA_KW);
}

bool is_lambda_node(GNode *node) {
    return !strcmp(((struct Unit *)node->data)->token->string, LAMBDA_KW);
}
bool is_association(struct Unit *c)
{
    return !strcmp(c->token->string, LET_KW);
}

bool
is_let(struct Unit *u)
{
    return !strcmp(u->token->string, LET_KW);
}

bool is_let2(struct Unit *u)
{
    if (u->toklen == LET_KW_SZ)
        return !strcmp(u->token->string, LET_KW);
    if (u->toklen > LET_KW_SZ && *(u->token->string + LET_KW_SZ) == ARITY_ID)
        return !strncmp(u->token->string, LET_KW, LET_KW_SZ);
    return false;
}

bool
is_lambda(struct Unit *u)
{
    return !strcmp(u->token->string, LAMBDA_KW) ||
        (u->toklen > LAMBDA_KW_SZ &&
         !strncmp(u->token->string, LAMBDA_KW, LAMBDA_KW_SZ) &&
         *(u->token->string + LAMBDA_KW_SZ) == ARITY_ID);
}

void set_lambda_type(struct Unit *u) /* Deprecated!!! */
{
    /* ich bin hier sicher dass u lambda ist */
    /* check minimum non-just-lambda kw construction: e.g. Lambda[:1&]
     * wo das 3te/letzte char & sein KÖNNTE! */
    if (u->toklen - LAMBDA_KW_SZ >= 3 &&
        *(u->token->string + (u->toklen - 1)) == REST_PARAM_ID)
        u->type = VARIADIC_LAMBDA;
    else
        u->type = UNIADIC_LAMBDA;
            
        
}

bool maybe_param_with_dflt_arg(struct Unit *u)
{
    return is_of_type(u, NAME) &&
        *u->token->string == PARAM_WITH_DFLT_ARG_ID;
}

bool
is_define(struct Unit *u)
{
    return !strcmp(u->token->string, DEFINEKW);
}

bool is_pass(struct Unit *u) {
    return !strcmp(u->token->string, FUNCALL_KEYWORD);
}

bool
starts_with(const char *a, const char *b)
{
   if (!strncmp(a, b, strlen(b))) return 1;
   return 0;
}

bool is_call(struct Unit *u)
{
    return starts_with(u->token->string, CALL_PRFX);
}
bool
is_call2(struct Unit *u)
{
    return !strcmp(u->token->string, CALL_KW) ||
        (u->toklen > CALL_KW_SZ &&
         !strncmp(u->token->string, CALL_KW, CALL_KW_SZ) &&
         *(u->token->string + CALL_KW_SZ) == ARITY_ID);
}


bool is_cpack(struct Unit *u) {
    return !strcmp(u->token->string, CPACK_KW);
}

bool is_cith(struct Unit *u) {
    return !strcmp(u->token->string, CITH_KW);
}
/* ********** math begin ********** */
bool
is_add_op(struct Unit *u)
{
    return !strcmp(u->token->string, ADDOPKW);
}
bool
is_add_op2(struct Unit *u)
{
    return !strcmp(u->token->string, ADD_OP_KW) ||
        (u->toklen > ADD_OP_KW_SZ &&
         !strncmp(u->token->string, ADD_OP_KW, ADD_OP_KW_SZ) &&
         *(u->token->string + ADD_OP_KW_SZ) == ARITY_ID);
    /* don't check the part after : could b an expression? */
}
bool
is_mul_op2(struct Unit *u)
{
    return !strcmp(u->token->string, MUL_OP_KW) ||
        (u->toklen > MUL_OP_KW_SZ &&
         !strncmp(u->token->string, MUL_OP_KW, MUL_OP_KW_SZ) &&
         *(u->token->string + MUL_OP_KW_SZ) == ARITY_ID);
    /* don't check the part after : could b an expression? */
}
bool
is_sub_op2(struct Unit *u)
{
    return !strcmp(u->token->string, SUB_OP_KW) ||
        (u->toklen > SUB_OP_KW_SZ &&
         !strncmp(u->token->string, SUB_OP_KW, SUB_OP_KW_SZ) &&
         *(u->token->string + SUB_OP_KW_SZ) == ARITY_ID);
    /* don't check the part after : could b an expression? */
}
bool
is_div_op2(struct Unit *u)
{
    return !strcmp(u->token->string, DIV_OP_KW) ||
        (u->toklen > DIV_OP_KW_SZ &&
         !strncmp(u->token->string, DIV_OP_KW, DIV_OP_KW_SZ) &&
         *(u->token->string + DIV_OP_KW_SZ) == ARITY_ID);
    /* don't check the part after : could b an expression? */
}

bool
is_sub_op(struct Unit *u)
{
    return !strcmp(u->token->string, SUBOPKW);
}
bool
is_div_op(struct Unit *u)
{
    return !strcmp(u->token->string, DIVOPKW);
}
bool
is_mul_op(struct Unit *u)
{
    return !strcmp(u->token->string, MULOPKW);
}

bool
is_exp_op(struct Unit *u)
{
    return !strcmp(u->token->string, EXPOPKW);
}
bool
is_inc_op(struct Unit *u)
{
    return !strcmp(u->token->string, INCOPKW);
}

bool
is_dec_op(struct Unit *u)
{
    return !strcmp(u->token->string, DECOPKW);
}

/* ********** math end ********** */

/* ******* hof begin ********* */
bool
is_lfold_op(struct Unit *u)
{
    return !strcmp(u->token->string, LFOLDOPKW);
}
bool
is_lfold_op2(struct Unit *u)
{
    return !strcmp(u->token->string, LFOLD_OP_KW) ||
        (u->toklen > LFOLD_OP_KW_SZ &&
         !strncmp(u->token->string, LFOLD_OP_KW, LFOLD_OP_KW_SZ) &&
         *(u->token->string + LFOLD_OP_KW_SZ) == ARITY_ID);
}


/* ******* hof end ********* */

/* ******** List begin ************ */
bool
is_nth_op(struct Unit *u)
{
    return !strcmp(u->token->string, NTHOPKW);
}

bool
is_list_op(struct Unit *u)
{
    return !strcmp(u->token->string, LIST_OP_KW);
}

bool
is_list_op2(struct Unit *u)
{
    return !strcmp(u->token->string, LIST_OP_KW) ||
        (u->toklen > LIST_OP_KW_SZ &&
         !strncmp(u->token->string, LIST_OP_KW, LIST_OP_KW_SZ) &&
         *(u->token->string + LIST_OP_KW_SZ) == ARITY_ID);
}

bool
is_size_op(struct Unit *u)
{
    return !strcmp(u->token->string, SIZEOPKW);
}
/* ******** list end *********** */

/* booleans */
bool is_true(struct Unit *u) {
    return !strcmp(u->token->string, TRUEKW);
}
bool is_false(struct Unit *u) {
    return !strcmp(u->token->string, FALSEKW);
}
bool is_bool(struct Unit *u) {
    return is_true(u) || is_false(u);
}

bool maybe_binding4(struct Unit *u) {
    char *str = u->token->string;
    return unit_type(u) == NAME && str[strlen(str) - 1] == BINDING_SUFFIX;
}

/* conditional */
bool
is_cond(struct Unit *u)
{
    return !strcmp(u->token->string, CONDKW);
}

bool
is_cond_if(struct Unit *u)
{
    return !strcmp(u->token->string, CONDIFKW);
}

bool
is_cond_then(struct Unit *u)
{
    return !strcmp(u->token->string, CONDTHENKW);
}

bool
is_cond_else(struct Unit *u)
{
    return !strcmp(u->token->string, CONDELSEKW);
}

/* **********mand/opt params */
bool
maybe_mand_param2(struct Unit *u)
{
    return is_of_type(u, NAME) &&
        *u->token->string != PARAM_WITH_DFLT_ARG_ID;       
    /* this is redicoulous! */
}
bool
maybe_mand_rest_param2(struct Unit *u)
{
    return is_of_type(u,NAME) &&
        *u->token->string == REST_PARAM_ID;
}
bool
maybe_opt_param2(struct Unit *u)
{
    return is_of_type(u, NAME) &&
        *u->token->string == PARAM_WITH_DFLT_ARG_ID;
}
bool
maybe_opt_rest_param2(struct Unit *u)
{
    return is_of_type(u,NAME) &&
        *u->token->string == PARAM_WITH_DFLT_ARG_ID &&
        *(u->token->string + 1) == REST_PARAM_ID;
}

/* deprecated */
bool
maybe_mand_param(struct Unit *u)
{
    char *str = u->token->string;
    return is_of_type(u, NAME) && /* unit is a kw */
        /* and last char is the suffix */
        *(str + (strlen(str) - 1)) == MAND_PARAM_SFX;
}

bool
maybe_opt_param(struct Unit *u)
{
    char *str = u->token->string;
    char *pos = strstr(str, OPT_PARAM_SFX);
    return is_of_type(u, NAME) && /* unit is a kw */
        /* and substr (opt param sfx) begins at the second char from
         * the end of str */
        ((size_t)(pos - str) == strlen(str) - 2) &&
        /* and after it comes nothing (is only 2 chars long) */
        strlen(pos) == 2;
}

bool
maybe_rest_mand_param(struct Unit *u)
{
    return *u->token->string == REST_PARAM_PFX &&
        maybe_mand_param(u);
}

bool
maybe_rest_opt_param(struct Unit *u)
{
    return *u->token->string == REST_PARAM_PFX &&
        maybe_opt_param(u);
        
}


/* ************************ */

bool maybe_pack_binding(struct Unit *u) {
    char *str = u->token->string;
    return unit_type(u) == NAME && /* ie not a numeric type */
        *str == PACK_BINDING_PREFIX &&
        str[strlen(str) - 1] == BINDING_SUFFIX;
        
}

bool
is_show_op(struct Unit *u)
{
    return !strcmp(u->token->string, SHOWOPKW);
}

bool need_block(struct Unit *u)
{
    return is_define(u) ||
        is_let(u) || is_let2(u) ||
        is_lambda4(u) || is_lambda(u) ||
        /* is_of_type(u, BINDING) || */
        /* is_of_type(u, PACK_BINDING) || */
        is_of_type(u, BOUND_BINDING) ||
        is_of_type(u, BOUND_PACK_BINDING) ||
        is_of_type(u, OPT_PARAM) || is_of_type(u, REST_OPT_PARAM) ||
        (u->type == CALL_OPT_REST_PARAM) ||
        is_show_op(u) ||
        /* is_pass(u) || */
        /* is_call(u) || */
        is_call2(u) ||
        /* is_cpack(u) || */
        is_list_op(u) || is_list_op2(u) ||
        is_nth_op(u) ||
        is_size_op(u) ||
        is_cond(u) ||
        is_cond_if(u) ||
        is_cond_then(u) ||
        is_cond_else(u) ||
        is_add_op(u) || is_add_op2(u) || is_mul_op2(u) || is_sub_op2(u) || is_div_op2(u) ||
        is_sub_op(u) ||
        is_mul_op(u) ||
        is_div_op(u) ||
        is_exp_op(u) ||
        is_inc_op(u) ||
        is_dec_op(u) ||
        is_lfold_op(u) || is_lfold_op2(u)
        ;
}


GNode *
find_enc_node_with_cap(GNode *node)
{
    do {
        node = node->parent;
        if (((struct Unit *)node->data)->max_cap != 0)
            return node;
    } while (node);
    return NULL;
}

GList *
find_enc_ulink(GList *ulink)
{
    do {
        ulink = ulink->prev;
        if (!((struct Unit *)ulink->data)->is_atomic) return ulink;
    } while (ulink->prev);
    return NULL;
}

void
digest_call(char *str, int *arg_cnt, int *rpt_cnt)
{
    char *arg_pfx_ptr = strchr(str, CALL_ARG_PFX);
    char *rpt_pfx_ptr = strchr(str, CALL_RPT_PFX);
    char *null_ptr = strchr(str, '\0');
    if (strlen(str) > 4) {      /* else an empty 'Call': stick to the defaults */
        if (arg_pfx_ptr && rpt_pfx_ptr) {
            if (rpt_pfx_ptr > arg_pfx_ptr) {
                char arg_buf[rpt_pfx_ptr - arg_pfx_ptr];
                strncpy(arg_buf, arg_pfx_ptr + 1, rpt_pfx_ptr - arg_pfx_ptr - 1);
                arg_buf[rpt_pfx_ptr - arg_pfx_ptr] = '\0';
                char rpt_buf[null_ptr - rpt_pfx_ptr];
                strncpy(rpt_buf, rpt_pfx_ptr + 1, null_ptr - rpt_pfx_ptr);
                /* if specified args num is '&' set args count to -2,
                 * this will be incremented back in eval_call
                 * resulting in the unlimited maxcap -1 */
                *arg_cnt = (strlen(arg_buf) == 1 && *arg_buf == CALL_ULTD_ARG_CHAR) ? -2 : atoi(arg_buf);
                *rpt_cnt = atoi(rpt_buf);
            } else {
                char rpt_buf[arg_pfx_ptr-rpt_pfx_ptr];
                strncpy(rpt_buf,rpt_pfx_ptr+1,arg_pfx_ptr-rpt_pfx_ptr-1);
                rpt_buf[arg_pfx_ptr-rpt_pfx_ptr]='\0';
                char arg_buf[null_ptr-arg_pfx_ptr];
                strncpy(arg_buf,arg_pfx_ptr+1,null_ptr-arg_pfx_ptr);
                *arg_cnt = (strlen(arg_buf) == 1 && *arg_buf == CALL_ULTD_ARG_CHAR) ? -2 : atoi(arg_buf);
                *rpt_cnt = atoi(rpt_buf);
            }
        } else if (arg_pfx_ptr) {
            char arg_buf[null_ptr-arg_pfx_ptr];
            strncpy(arg_buf, arg_pfx_ptr + 1, null_ptr - arg_pfx_ptr);
            *arg_cnt = (strlen(arg_buf) == 1 && *arg_buf == CALL_ULTD_ARG_CHAR) ? -2 : atoi(arg_buf);
        } else if (rpt_pfx_ptr) {
            char rpt_buf[null_ptr - rpt_pfx_ptr];
            strncpy(rpt_buf, rpt_pfx_ptr + 1, null_ptr - rpt_pfx_ptr);
            *rpt_cnt = atoi(rpt_buf);                    
        }                
    }
}

void
digest_arid(char *s, int *ar)
{
    char *sd = strdup(s);
    char *sdc = sd;
    strtok(sd, ".");/* wegschmeißen */
    char *x = strtok(NULL, ".");
    *ar = atoi(x);
    free(sdc);
}

void set_let_max_cap(struct Unit *u)
{
    if (u->toklen == LET_KW_SZ)
        /* only the let expression; does this make sense??! */
        u->max_cap = 1;
    else if (u->toklen > LET_KW_SZ)
        u->max_cap = atoi(u->token->string + LET_KW_SZ + 1) * 2 + 1;
}

void
set_lfold_op_maxcap(struct Unit *u)
{
    if (u->toklen == LFOLD_OP_KW_SZ)
        /* function, accumulator, list */
        u->max_cap = 1 + 1 + 1;
    else if (u->toklen > LFOLD_OP_KW_SZ)
        /* function, accumulator, list1...listN */
        u->max_cap = 1 + 1 + atoi(u->token->string + LFOLD_OP_KW_SZ + 1);
}

void set_lambda_max_cap(struct Unit *u)
{
    if (u->toklen == LAMBDA_KW_SZ) {
        u->max_cap = 1;         /* 0-ary */
        u->type = UNIADIC_LAMBDA;
    }
    else if (u->toklen > LAMBDA_KW_SZ) {
        if (*(u->token->string + (u->toklen - 1)) == REST_PARAM_ID) {
            u->type = VARIADIC_LAMBDA;
            char *nparams = strndup(u->token->string + LAMBDA_KW_SZ + 1, strlen(u->token->string + LAMBDA_KW_SZ + 1 - 1));
            u->max_cap = atoi(nparams) + 1;
            free(nparams);
        } else {
            u->type = UNIADIC_LAMBDA;
            u->max_cap = atoi(u->token->string + LAMBDA_KW_SZ + 1) + 1;
        }   
    }
}
void
set_add_op_maxcap(struct Unit *u)
{
    if (!strcmp(u->token->string, ADD_OP_KW))
        u->max_cap = 2;         /* Add denotes a binary operation */
    else if (u->toklen > ADD_OP_KW_SZ && *(u->token->string + ADD_OP_KW_SZ) == ARITY_ID)
        u->max_cap = atoi(u->token->string + ADD_OP_KW_SZ + 1);
}
void
set_mul_op_maxcap(struct Unit *u)
{
    if (!strcmp(u->token->string, MUL_OP_KW))
        u->max_cap = 2;         /* denotes a binary operation */
    else if (u->toklen > MUL_OP_KW_SZ && *(u->token->string + MUL_OP_KW_SZ) == ARITY_ID)
        u->max_cap = atoi(u->token->string + MUL_OP_KW_SZ + 1);
}
void
set_sub_op_maxcap(struct Unit *u)
{
    if (!strcmp(u->token->string, SUB_OP_KW))
        u->max_cap = 2;         /* denotes a binary operation */
    else if (u->toklen > SUB_OP_KW_SZ && *(u->token->string + SUB_OP_KW_SZ) == ARITY_ID)
        u->max_cap = atoi(u->token->string + SUB_OP_KW_SZ + 1);
}
void
set_div_op_maxcap(struct Unit *u)
{
    if (!strcmp(u->token->string, DIV_OP_KW))
        u->max_cap = 2;         /* denotes a binary operation */
    else if (u->toklen > DIV_OP_KW_SZ && *(u->token->string + DIV_OP_KW_SZ) == ARITY_ID)
        u->max_cap = atoi(u->token->string + DIV_OP_KW_SZ + 1);
}


void set_call_max_cap(struct Unit *u)
{
    if (!strcmp(u->token->string, CALL_KW))
        u->max_cap = 1;         /* only func */
    else if (u->toklen > CALL_KW_SZ && *(u->token->string + CALL_KW_SZ) == ARITY_ID)
        u->max_cap = atoi(u->token->string + CALL_KW_SZ + 1) + 1; /* letztes +1 ist das ELSE  */
}
void set_listop_maxcap(struct Unit *u)
{
    if (!strcmp(u->token->string, LIST_OP_KW))
        u->max_cap = 0;         /* empty list */
    else if (u->toklen > LIST_OP_KW_SZ && *(u->token->string + LIST_OP_KW_SZ) == ARITY_ID)
        u->max_cap = atoi(u->token->string + LIST_OP_KW_SZ + 1);
}
/* goes through the atoms, root will be the container with tl_cons ... */
GNode *parse3(GList *ulink)
{    
    GList *tl_ulink = ulink;      /* toplevel unit link */
    GNode *root = g_node_new((struct Unit *)tl_ulink->data);
    ulink = ulink->next;
    /* effective binding units are units which introduce bindings,
       e.g. lambda, let, pass */
    struct Unit *curr_bind_unit = NULL; /* current binding unit */
    GNode *enc_node = NULL;              /* enclosing node */
    /* struct Unit *eff_opt_param_unit = NULL; /\* effective optional param unit *\/ */
    while (ulink) {
        
        /* /\* 1.1. first attempt to find the enclosing node of this unit link *\/ */
        /* if (eff_opt_param_unit && is_enclosed_in5((struct Unit *)ulink->data, eff_opt_param_unit)) { */
        /*     enc_node = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, eff_opt_param_unit); */
        /*     eff_opt_param_unit=NULL; */
        /* } */
        /* else if ( */
        /*     ( */
        /*         maybe_mand_param2((struct Unit *)ulink->data) || */
        /*         maybe_opt_param2((struct Unit *)ulink->data) */
        /*         ) && */
        /*     curr_bind_unit && */
        /*     is_enclosed_in5((struct Unit *)ulink->data, curr_bind_unit)) */
        /*     /\* then enc node is the current binding unit *\/ */
        /*     enc_node = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, curr_bind_unit); */
        /* else */
        /*     enc_node = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, */
        /*                            (struct Unit *)find_enc_ulink(ulink)->data); */
        
        /* 1.2. if the computed enclosing node has no more capacity set the
           closest parent of it with capacity to be the enclosing node */
        enc_node = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL,
                               (struct Unit *)find_enc_ulink(ulink)->data);
        if (((struct Unit *)enc_node->data)->max_cap == 0)
            enc_node = find_enc_node_with_cap(enc_node);
        
        /* 2. establish definite binding types based on the enclosing
         * units and the unit's look */        
        if (is_lambda((struct Unit *)enc_node->data) && ((struct Unit *)enc_node->data)->max_cap > 1) {
            /* Lambda:3 a s d body */
            if (maybe_mand_rest_param2((struct Unit *)ulink->data)) {
                ((struct Unit *)ulink->data)->type = REST_MAND_PARAM;
                ((struct Unit *)enc_node->data)->type = VARIADIC_LAMBDA; /* aaaah, war doch nicht uniadic! */
            }  else if (maybe_opt_rest_param2((struct Unit *)ulink->data)) {
                ((struct Unit *)ulink->data)->type = REST_OPT_PARAM;
                ((struct Unit *)enc_node->data)->type = VARIADIC_LAMBDA;
            } else if (maybe_mand_param2((struct Unit *)ulink->data)) {
                ((struct Unit *)ulink->data)->type = MAND_PARAM;
                /* ((struct Unit *)enc_node->data)->type = UNIADIC_LAMBDA; */
            }  else if (maybe_opt_param2((struct Unit *)ulink->data)) {
                ((struct Unit *)ulink->data)->type = OPT_PARAM;
                /* ((struct Unit *)enc_node->data)->type = UNIADIC_LAMBDA; */
            }
        } else if (is_call2((struct Unit *)enc_node->data)) {
             if (maybe_opt_rest_param2((struct Unit *)ulink->data)) {
                 ((struct Unit *)ulink->data)->type = CALL_OPT_REST_PARAM;
            } else if (maybe_opt_param2((struct Unit *)ulink->data)) {
                ((struct Unit *)ulink->data)->type = OPT_PARAM;
            }
        }

        /* 3. decrement maximum capacity of the enclosing node with definite
         * amount of capacity if the unit takes up it's capacity */
        if (is_of_type((struct Unit *)enc_node->data, BOUND_BINDING) ||
            is_of_type((struct Unit *)enc_node->data, BOUND_PACK_BINDING) ||
            is_define((struct Unit *)enc_node->data) ||
            is_show_op((struct Unit *)enc_node->data) ||
            is_size_op((struct Unit *)enc_node->data) ||
            is_nth_op((struct Unit *)enc_node->data) ||
            /* is_list_op((struct Unit *)enc_node->data) || */
            /* is_call((struct Unit *)enc_node->data) || */
            is_cond_if((struct Unit *)enc_node->data) ||
            is_cond_then((struct Unit *)enc_node->data) ||
            is_cond_else((struct Unit *)enc_node->data) ||
            is_add_op((struct Unit *)enc_node->data) ||
            is_add_op2((struct Unit *)enc_node->data) ||
            is_mul_op2((struct Unit *)enc_node->data) ||
            is_sub_op2((struct Unit *)enc_node->data) ||
            is_div_op2((struct Unit *)enc_node->data) ||
            is_sub_op((struct Unit *)enc_node->data) ||
            is_mul_op((struct Unit *)enc_node->data) ||
            is_div_op((struct Unit *)enc_node->data) ||
            is_lfold_op((struct Unit *)enc_node->data) ||
            is_exp_op((struct Unit *)enc_node->data) ||
            is_inc_op((struct Unit *)enc_node->data) ||
            is_dec_op((struct Unit *)enc_node->data) ||
            
            is_let2((struct Unit *)enc_node->data) || is_lfold_op2((struct Unit *)enc_node->data) ||
            is_lambda((struct Unit *)enc_node->data) ||
            (is_call2((struct Unit *)enc_node->data) && ((struct Unit *)ulink->data)->type != CALL_OPT_REST_PARAM) ||
            is_list_op2((struct Unit *)enc_node->data) ||
            is_of_type((struct Unit *)enc_node->data, OPT_PARAM) ||
            is_of_type((struct Unit *)enc_node->data, REST_OPT_PARAM)
            ) {
            if (((struct Unit *)enc_node->data)->max_cap)
                ((struct Unit *)enc_node->data)->max_cap--;
            else {
                /* if the previous enclosing node's capacity is
                 * exhausted (i.e. is 0), look for a top node with capa to be the
                 * new enclosing node */
                enc_node = find_enc_node_with_cap(enc_node);
            }
        } else if (is_of_type((struct Unit *)enc_node->data, CALL_OPT_REST_PARAM)) {
            if (((struct Unit *)enc_node->data)->max_cap) {
                ((struct Unit *)enc_node->data)->max_cap--;
                ((struct Unit *)enc_node->parent->data)->max_cap--; /* maxcap of Callesh r ham biar paiin */
            }
                
            else {
                /* if the previous enclosing node's capacity is
                 * exhausted (i.e. is 0), look for a top node with capa to be the
                 * new enclosing node */
                enc_node = find_enc_node_with_cap(enc_node);
            }
        }
        
        /* 4. set the maximum absorption capacity for this unit */
        /* if (is_of_type((struct Unit *)ulink->data, BOUND_BINDING)) */
        /*     ((struct Unit *)ulink->data)->max_cap = 1; */
        if (is_of_type((struct Unit *)ulink->data, OPT_PARAM) ||
            is_of_type((struct Unit *)ulink->data, REST_OPT_PARAM))
            ((struct Unit *)ulink->data)->max_cap = 1;
        else if (((struct Unit *)ulink->data)->type == CALL_OPT_REST_PARAM)
            ((struct Unit *)ulink->data)->max_cap = ((struct Unit *)enc_node->data)->max_cap; /* maxcap of it's Call */
        else if (is_define((struct Unit *)ulink->data)) {
            /* capa = name, data */
            ((struct Unit *)ulink->data)->max_cap = 2;
        } else if (is_cith((struct Unit *)ulink->data)) {
            /* captures: index  pack */
            ((struct Unit *)ulink->data)->max_cap = 2;
        } else if (is_show_op((struct Unit *)ulink->data)) {
            /* captures: a single data */
            ((struct Unit *)ulink->data)->max_cap = 1;
        } else if (is_size_op((struct Unit *)ulink->data))
            /* capa = list */
            ((struct Unit *)ulink->data)->max_cap = 1;
        else if (is_nth_op((struct Unit *)ulink->data))
            /* captures: nth, list */
            ((struct Unit *)ulink->data)->max_cap = 2;
        else if (is_list_op((struct Unit *)ulink->data))
            /* no capa, List is used ONLY as default arg to
             * list's &ITEMS:= param! */
            ((struct Unit *)ulink->data)->max_cap = 0;
        /* else if (is_call((struct Unit *)ulink->data)) { */
        /*     /\* capa = the specified number of args to the function *\/ */
        /*     int arg_cnt = 0, rpt_cnt = 1; /\* args count, repeatitions count *\/ */
        /*     digest_call(((struct Unit *)ulink->data)->token->string, &arg_cnt, &rpt_cnt); */
        /*     ((struct Unit *)ulink->data)->max_cap = arg_cnt + 1; /\* arg_cnt = args, + 1 = fnc name *\/ */
        /*     ((struct Unit *)ulink->data)->call_rpt_cnt = rpt_cnt; */
        /* } */ else if (is_cond_if((struct Unit *)ulink->data) ||
                   is_cond_then((struct Unit *)ulink->data))
            /* capa = expression */
            ((struct Unit *)ulink->data)->max_cap = 1;
        else if (is_cond_else((struct Unit *)ulink->data)) {
            ((struct Unit *)ulink->data)->max_cap = 1;
            ((struct Unit *)enc_node->data)->max_cap = 0; /* suddenly, no decrementing! */
        } else if (is_add_op((struct Unit *)ulink->data) ||
                   is_sub_op((struct Unit *)ulink->data) ||
                   is_mul_op((struct Unit *)ulink->data) ||
                   is_div_op((struct Unit *)ulink->data) ||
                   is_exp_op((struct Unit *)ulink->data))
            /* captures: first number, second number */
            ((struct Unit *)ulink->data)->max_cap = 2;
        else if (is_inc_op((struct Unit *)ulink->data) ||
                 is_dec_op((struct Unit *)ulink->data))
            /* captures: a number */
            ((struct Unit *)ulink->data)->max_cap = 1;
        else if (is_lfold_op((struct Unit *)ulink->data))
            /* captures: id, list, fn */
            ((struct Unit *)ulink->data)->max_cap = 3;
        else if (is_let2((struct Unit *)ulink->data))
            set_let_max_cap((struct Unit *)ulink->data);
        else if (is_lambda((struct Unit *)ulink->data)) {
            set_lambda_max_cap((struct Unit *)ulink->data);
            ((struct Unit *)ulink->data)->type = UNIADIC_LAMBDA; /* Wir gehen einfach erstmal davon aus!!! */
            /* set_lambda_type((struct Unit *)ulink->data); */
        }            
        else if (is_call2((struct Unit *)ulink->data)) {
            set_call_max_cap((struct Unit *)ulink->data);
        } else if (is_list_op2((struct Unit *)ulink->data)) {
            set_listop_maxcap((struct Unit *)ulink->data);
        }
        else if (is_add_op2((struct Unit *)ulink->data))
            set_add_op_maxcap((struct Unit *)ulink->data);
        else if (is_mul_op2((struct Unit *)ulink->data))
            set_mul_op_maxcap((struct Unit *)ulink->data);
        else if (is_sub_op2((struct Unit *)ulink->data))
            set_sub_op_maxcap((struct Unit *)ulink->data);
        else if (is_div_op2((struct Unit *)ulink->data))
            set_div_op_maxcap((struct Unit *)ulink->data);
        else if (is_lfold_op2((struct Unit *)ulink->data))
            set_lfold_op_maxcap((struct Unit *)ulink->data);
        
         
        
        if (need_block((struct Unit *)ulink->data)) {
            ((struct Unit *)ulink->data)->is_atomic = false;            
            /* new block has it's own environment */
            ((struct Unit *)ulink->data)->env = g_hash_table_new(g_str_hash, g_str_equal);
            /* set current binding unit (if it makes bindings bing: boundbinf:= etc.) */
            if (is_lambda4((struct Unit *)ulink->data) ||
                is_call2((struct Unit *)ulink->data) ||
                is_let((struct Unit *)ulink->data) ||
                is_let2((struct Unit *)ulink->data) ||
                is_lambda((struct Unit *)ulink->data))
                curr_bind_unit = (struct Unit *)ulink->data;            
        } else {			/* an atomic unit */
            ((struct Unit *)ulink->data)->is_atomic = true;
            ((struct Unit *)ulink->data)->max_cap = 0;
            /* is the unit true or false? set it's boolean type */
            if (is_bool(((struct Unit *)ulink->data)))
                ((struct Unit *)ulink->data)->type = BOOL;
        }
        
        g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, enc_node->data),
                      -1,
                      g_node_new((struct Unit *)ulink->data));

        /* when the unit's enc_node is a lambda/let, the unit is the first item
           of lambda and not a binding form, the unit is taken to be the final
           expression of lambda (i.e. lambda will be closed!) */
        if (((is_lambda4((struct Unit *)enc_node->data) ||
              is_let((struct Unit *)enc_node->data)) &&
             unit_type((struct Unit *)ulink->data) != BINDING &&
             unit_type((struct Unit *)ulink->data) != BOUND_BINDING &&
             unit_type((struct Unit *)ulink->data) != PACK_BINDING &&
             unit_type((struct Unit *)ulink->data) != BOUND_PACK_BINDING)) {
            ((struct Unit *)enc_node->data)->max_cap = 0;
            curr_bind_unit = NULL;
        }
        
        /* process next unit */
        ulink = ulink->next;
    }
    return root;
}





void
assert_func_param(GNode *node, GNode *last_child)
{
    if (node != last_child &&
        unit_type((struct Unit *)node->data) != BINDING &&
        unit_type((struct Unit *)node->data) != BOUND_BINDING &&
        unit_type((struct Unit *)node->data) != PACK_BINDING &&
        unit_type((struct Unit *)node->data) != BOUND_PACK_BINDING) {
        fprintf(stderr, "invalid thing in place of parameter\n");
        print_node(node, NULL);
        exit(EXIT_FAILURE);
    }
}

/* gboolean sanify_lambda(GNode *node, gpointer data) { */
/*     if (is_lambda_node(node)) { */
/*         if (g_node_n_children(node)) { */
/*             GNode *last_child = g_node_last_child(node); */
/*             /\* first assert that every child node except with the last one */
/*                is a binding (ie a parameter decleration) *\/ */
/*             g_node_children_foreach(node, G_TRAVERSE_ALL, */
/*                                     (GNodeForeachFunc)assert_func_param, */
/*                                     last_child); */
/*             /\* if the last node is a mandatory parameter (i.e. a parameter */
/*                without default value, which i could take out as the lambda */
/*                expression), it is an error! *\/ */
/*             if (((struct Unit *)last_child->data)->type == BINDING || */
/*                 unit_type((struct Unit *)last_child->data) == PACK_BINDING) { */
/*                 fprintf(stderr, "binding '%s' kann nicht ende von deinem lambda sein\n", */
/*                         ((struct Unit *)last_child->data)->token->string); */
/*                 exit(EXIT_FAILURE); */
/*                 /\* if the last node LOOKS LIKE a parameter with a default */
/*                    argument (i.e. optional argument/bound binding), we treat */
/*                    it's default argument as the final expression of lambda *\/ */
/*             } else if (unit_type((struct Unit *)last_child->data) == BOUND_BINDING || */
/*                        unit_type((struct Unit *)last_child->data) == BOUND_PACK_BINDING) { */
/*                 GNode *bound_value = g_node_last_child(last_child); */
/*                 g_node_unlink(bound_value); */
/*                 g_node_insert(node, -1, bound_value); */
/*                 /\* and reset the parameter types to their mandatory versions *\/ */
/*                 if (unit_type((struct Unit *)last_child->data) == BOUND_BINDING) */
/*                     ((struct Unit *)last_child->data)->type = BINDING; */
/*                 /\* if we took the last and only child of the bound pack */
/*                    binding out as the lambda expression and it has no */
/*                    children, change it's type to a simple pack binding without */
/*                    default arguments *\/ */
/*                 else if (g_node_n_children(last_child) == 0) */
/*                     ((struct Unit *)last_child->data)->type = PACK_BINDING; */
/*             } */
/*         } else { */
/*             fprintf(stderr, "malformed lambda\n"); */
/*             print_node(node, NULL); */
/*             fprintf(stderr, "missing expression\n"); */
/*             exit(EXIT_FAILURE); */
/*         } */
/*     } */
/*     return false; */
/* } */

/* void post_parse_lambda_check(GNode *root) { */
/*     puts("parse-time lambda sanify"); */
/*     g_node_traverse(root, G_PRE_ORDER, */
/*                     G_TRAVERSE_ALL, -1, */
/*                     (GNodeTraverseFunc)sanify_lambda, NULL); */
/*     puts("  lambdas sanified"); */
/* } */


/* schmelz die 2 zusammen mit der obigen */
void assert_bound_binding_node(GNode *node, GNode *last_child) {
    /* e.g. a parameter or a binding in let */
    if (node != last_child &&
        !is_of_type((struct Unit *)node->data, BOUND_BINDING)) {
        fprintf(stderr, "bound binding assertion failed\n");
        print_node(node, NULL);
        fprintf(stderr, "not bound\n");
        exit(EXIT_FAILURE);
    }
}

void
assert_call_param(GNode *binding, GNode *func_node)
{
    if ((binding != func_node && unit_type((struct Unit *)binding->data) == BINDING)) {
        fprintf(stderr, "malformed argument passed\n");
        print_node(binding, NULL);
        fprintf(stderr, "is not bound\n");
        exit(EXIT_FAILURE);
    }
}

gboolean check_pass(GNode *node, gpointer data) {
    if (is_call((struct Unit *)node->data)) {
        if (g_node_n_children(node)) {
            /* GNode *func_node = g_node_last_child(node); */
            GNode *func_node = g_node_nth_child(node, 0);
            /* if (unit_type((struct Unit *)func_node->data) != NAME) { */
            /*     fprintf(stderr, "malformed pass\n"); */
            /*     print_node(func_node, NULL); */
            /*     fprintf(stderr, "is not a function\n"); */
            /*     exit(EXIT_FAILURE); */
            /* } */
            g_node_children_foreach(node, G_TRAVERSE_ALL,
                                    (GNodeForeachFunc)assert_call_param,
                                    func_node);
        } else {
            fprintf(stderr, "pass braucht mind. eine expression: func!\n");
            exit(EXIT_FAILURE);
        }
    }
    return false;
}

void post_parse_call_check(GNode *root) {
    puts("parse-time call check");
    g_node_traverse(root, G_PRE_ORDER,
                    G_TRAVERSE_ALL, -1,
                    (GNodeTraverseFunc)check_pass, NULL);
    puts("  call checked");
}



void check_let_binding(GNode *node, GNode *last_child) {
    if (node == last_child) {
        if (is_of_type((struct Unit *)node->data, BINDING) ||
            is_of_type((struct Unit *)node->data, BOUND_BINDING)) {
            fprintf(stderr, "let final expression cant be a binding\n");
            print_node(node, NULL);
            exit(EXIT_FAILURE);
        }
    } else {                    /* bindings */
        if (!is_of_type((struct Unit *)node->data, BOUND_BINDING)) {
            fprintf(stderr, "let binding not bound\n");
            print_node(node, NULL);
            exit(EXIT_FAILURE);                        
        }
    }
}

gboolean check_let(GNode *node, gpointer data) {
    if (is_let((struct Unit *)node->data)) {
        GNode *last_child = g_node_last_child(node);
        /* check that there is something in let */
        if (g_node_n_children(node) < 2) {
            fprintf(stderr, "malformed let, missing bindings or expr\n");
            print_node(node, NULL);
            exit(EXIT_FAILURE);            
        }
        g_node_children_foreach(node, G_TRAVERSE_ALL,
                                (GNodeForeachFunc)check_let_binding,
                                last_child);
    }
    return false;
}

void post_parse_let_check(GNode *node) {
    puts("parse-time let check");
    g_node_traverse(node, G_PRE_ORDER,
                    G_TRAVERSE_ALL, -1,
                    (GNodeTraverseFunc)check_let, NULL);
    puts("  assocs checked");
}
