#include <stdlib.h>		/* EXIT_FAILURE */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
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
        struct Unit *u1 = (unitp_t)ulink1->data, *u2 = (unitp_t)ulink2->data;
        return u1->token.col_start_idx > u2->token.col_start_idx &&
            u1->token.line >= u2->token.line;
    } else {
        return false;
    }
}

bool is_enclosed_in4(struct Unit *a1, struct Unit *a2)
{
    return a1->token.col_start_idx > a2->token.col_start_idx &&
        a1->token.line >= a2->token.line;
}

bool
is_enclosed_in5(struct Unit *a1, struct Unit *a2)
{
    if (a1->token.line == a2->token.line) {
        return a1->token.col_start_idx > a2->token.col_start_idx;
    } else if (a1->token.line > a2->token.line) {
        return true;
    } else return false;
}

int bottom_line_number3(GList *list)
{
    int line = -1;
    while (list) {
        if (((unitp_t)list->data)->token.line > line)
            line = ((unitp_t)list->data)->token.line;
        list = list->next;
    }
    return line;
}

GList *enclosures3(GList *ulink, GList *list) {
    GList *sll = NULL;
    for (;
         list && ((unitp_t)list->data)->uuid < ((unitp_t)ulink->data)->uuid;
         list = list->next) {
        if (!((unitp_t)list->data)->is_atomic && is_enclosed_in3(ulink, list))
            sll = g_list_append(sll, list->data);
    }
    return sll;
}

/* remove from encs */
GList *bottom_enclosures3(GList *encs) {
    int bot_linum = bottom_line_number3(encs);
    GList *out = NULL;
    while (encs) {
        if (((unitp_t)encs->data)->token.line == bot_linum)
            out = g_list_append(out, encs->data);
        encs = encs->next;
    }
    return out;
}
GList *rightmost_enclosure(GList *botencs) {
    int col_start_idx = LEAST_COL_START_IDX;
    GList *rmost = NULL;
    while (botencs) {
        if (((unitp_t)botencs->data)->token.col_start_idx > col_start_idx) {
            col_start_idx = ((unitp_t)botencs->data)->token.col_start_idx;
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
    return !strcmp(u->token.str, FUNCTION_KW);
}


bool is_lambda_node(GNode *node) {
    return !strcmp(((unitp_t)node->data)->token.str, FUNCTION_KW);
}
bool is_association(struct Unit *c)
{
    return !strcmp(c->token.str, LET_KW);
}

bool
is_let(struct Unit *u)
{
    return !strcmp(u->token.str, LET_KW);
}

bool
is_define(struct Unit *u)
{
    return !strcmp(u->token.str, DEFINE_KW);
}

bool is_pass(struct Unit *u) {
    return !strcmp(u->token.str, FUNCALL_KEYWORD);
}

bool starts_with(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

bool
is_call(struct Unit *u)
{
    return starts_with(u->token.str, CALL_PRFX);
}


bool is_cpack(struct Unit *u) {
    return !strcmp(u->token.str, CPACK_KW);
}

bool is_cith(struct Unit *u) {
    return !strcmp(u->token.str, CITH_KW);
}

/* ******** List begin ************ */
bool
is_tila_nth(struct Unit *u)
{
    return !strcmp(u->token.str, TILA_NTH_KW);
}

bool
is_tila_list(struct Unit *u)
{
    return !strcmp(u->token.str, TILA_LIST_KW);
}

bool
is_tila_size(struct Unit *u)
{
    return !strcmp(u->token.str, TILA_SIZE_KW);
}
/* ******** list end *********** */

/* booleans */
bool is_true(struct Unit *u) {
    return !strcmp(u->token.str, TRUE_KW);
}
bool is_false(struct Unit *u) {
    return !strcmp(u->token.str, FALSE_KW);
}
bool is_bool(struct Unit *u) {
    return is_true(u) || is_false(u);
}

bool maybe_binding4(struct Unit *u) {
    char *str = u->token.str;
    return unit_type(u) == NAME && str[strlen(str) - 1] == BINDING_SUFFIX;
}

/* conditional */
bool
is_cond(struct Unit *u)
{
    return !strcmp(u->token.str, COND_KW);
}

bool
is_cond_if(struct Unit *u)
{
    return !strcmp(u->token.str, COND_IF_KW);
}

bool
is_cond_then(struct Unit *u)
{
    return !strcmp(u->token.str, COND_THEN_KW);
}

bool
is_cond_else(struct Unit *u)
{
    return !strcmp(u->token.str, COND_ELSE_KW);
}

/* **********mand/opt params */
bool
maybe_mand_param(struct Unit *u)
{
    char *str = u->token.str;
    return is_of_type(u, NAME) && /* unit is a kw */
        /* and last char is the suffix */
        *(str + (strlen(str) - 1)) == MAND_PARAM_SFX;
}

bool
maybe_opt_param(struct Unit *u)
{
    char *str = u->token.str;
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
    return *u->token.str == REST_PARAM_PFX &&
        maybe_mand_param(u);
}

bool
maybe_rest_opt_param(struct Unit *u)
{
    return *u->token.str == REST_PARAM_PFX &&
        maybe_opt_param(u);
        
}


/* ************************ */

bool maybe_pack_binding(struct Unit *u) {
    char *str = u->token.str;
    return unit_type(u) == NAME && /* ie not a numeric type */
        *str == PACK_BINDING_PREFIX &&
        str[strlen(str) - 1] == BINDING_SUFFIX;
        
}

bool
is_tila_show(struct Unit *u)
{
    return !strcmp(u->token.str, TILA_SHOW_KW);
}

bool
need_block(struct Unit *u)
{
    return is_define(u) ||
        is_let(u) ||
        is_lambda4(u) ||
        /* is_of_type(u, BINDING) || */
        /* is_of_type(u, PACK_BINDING) || */
        is_of_type(u, BOUND_BINDING) ||
        is_of_type(u, BOUND_PACK_BINDING) ||
        is_tila_show(u) ||
        /* is_pass(u) || */
        /* is_call(u) || */
        is_call(u) ||
        /* is_cpack(u) || */
        is_tila_list(u) ||
        /* is_cith(u) || */
        is_tila_nth(u) ||
        is_tila_size(u) ||
        is_cond(u) ||
        is_cond_if(u) ||
        is_cond_then(u) ||
        is_cond_else(u)
        ;
}


GNode *
find_enc_node_with_cap(GNode *node)
{
    do {
        node = node->parent;
        if (((unitp_t)node->data)->max_cap != 0)
            return node;
    } while (node);
    return NULL;
}
GList *
find_enc_ulink(GList *ulink)
{
    do {
        ulink = ulink->prev;
        if (!((unitp_t)ulink->data)->is_atomic)
            return ulink;
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
                arg_buf[rpt_pfx_ptr-arg_pfx_ptr]='\0';
                char rpt_buf[null_ptr-rpt_pfx_ptr];
                strncpy(rpt_buf,rpt_pfx_ptr+1,null_ptr-rpt_pfx_ptr);
                *arg_cnt = atoi(arg_buf);
                *rpt_cnt = atoi(rpt_buf);
            } else {
                char rpt_buf[arg_pfx_ptr-rpt_pfx_ptr];
                strncpy(rpt_buf,rpt_pfx_ptr+1,arg_pfx_ptr-rpt_pfx_ptr-1);
                rpt_buf[arg_pfx_ptr-rpt_pfx_ptr]='\0';
                char arg_buf[null_ptr-arg_pfx_ptr];
                strncpy(arg_buf,arg_pfx_ptr+1,null_ptr-arg_pfx_ptr);
                *arg_cnt = atoi(arg_buf);
                *rpt_cnt = atoi(rpt_buf);
            }
        } else if (arg_pfx_ptr) {
            char arg_buf[null_ptr-arg_pfx_ptr];
            strncpy(arg_buf, arg_pfx_ptr + 1, null_ptr - arg_pfx_ptr);
            *arg_cnt = atoi(arg_buf);                    
        } else if (rpt_pfx_ptr) {
            char rpt_buf[null_ptr - rpt_pfx_ptr];
            strncpy(rpt_buf, rpt_pfx_ptr + 1, null_ptr - rpt_pfx_ptr);
            *rpt_cnt = atoi(rpt_buf);                    
        }                
    }

}

/* goes through the atoms, root will be the container with tl_cons ... */
GNode *
parse3(GList *ulink)
{    
    GList *tl_ulink = ulink;      /* toplevel unit link */
    ulink = ulink->next;
    GNode *root = g_node_new((unitp_t)tl_ulink->data);
    /* effective binding units are units which introduce bindings,
       e.g. lambda, let, pass */
    struct Unit *curr_bind_unit = NULL; /* current binding unit */
    GNode *enc_node;              /* enclosing node */
    
    while (ulink) {
        
        /* 1.1. first attempt to find the enclosing node of this unit link */
        if ((maybe_rest_mand_param((unitp_t)ulink->data) ||
             maybe_rest_opt_param((unitp_t)ulink->data) ||
             maybe_mand_param((unitp_t)ulink->data) ||
             maybe_opt_param((unitp_t)ulink->data)) &&
            curr_bind_unit &&
            is_enclosed_in5((unitp_t)ulink->data, curr_bind_unit))
            /* then enc node is the current binding unit */
            enc_node = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, curr_bind_unit);
        else
            enc_node = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL,
                                   (unitp_t)find_enc_ulink(ulink)->data);
        /* 1.2. if the computed enclosing node has no more capacity set the
           closest parent of it with capacity to be the enclosing node */
        if (((unitp_t)enc_node->data)->max_cap == 0)
            enc_node = find_enc_node_with_cap(enc_node);
        
        /* 2. establish definite binding types based on the enclosing
         * units and the unit's look */
        if (is_let((unitp_t)enc_node->data) ||
            is_lambda4((unitp_t)enc_node->data) ||
            is_call((unitp_t)enc_node->data)) {
            if (maybe_rest_mand_param((unitp_t)ulink->data))
                ((unitp_t)ulink->data)->type = PACK_BINDING;
            else if (maybe_rest_opt_param((unitp_t)ulink->data)) {
                ((unitp_t)ulink->data)->type = BOUND_PACK_BINDING;
            } else if (maybe_mand_param((unitp_t)ulink->data)) {
                ((unitp_t)ulink->data)->type = BINDING;
            } else if (maybe_opt_param((unitp_t)ulink->data)) {
                ((unitp_t)ulink->data)->type = BOUND_BINDING;                
            }
        }

        /* 3. decrement maximum capacity of the enclosing node with definite
         * amount of capacity if the unit takes up it's capacity */
        if (is_of_type((unitp_t)enc_node->data, BOUND_BINDING) ||
            is_of_type((unitp_t)enc_node->data, BOUND_PACK_BINDING) ||
            is_define((unitp_t)enc_node->data) ||
            is_tila_show((unitp_t)enc_node->data) ||
            is_tila_size((unitp_t)enc_node->data) ||
            is_tila_nth((unitp_t)enc_node->data) ||
            /* is_tila_list((unitp_t)enc_node->data) || */
            is_call((unitp_t)enc_node->data) ||
            is_cond_if((unitp_t)enc_node->data) ||
            is_cond_then((unitp_t)enc_node->data) ||
            is_cond_else((unitp_t)enc_node->data)
            ) {
            if (((unitp_t)enc_node->data)->max_cap)
                ((unitp_t)enc_node->data)->max_cap--;
            else
                /* if the previous enclosing node's capacity is
                 * exhausted (i.e. is 0), look for a top node with capa to be the
                 * new enclosing node */
                enc_node = find_enc_node_with_cap(enc_node);
        }
        
        /* 4. set the maximum absorption capacity for this unit */
        if (is_of_type((unitp_t)ulink->data, BOUND_BINDING))
            ((unitp_t)ulink->data)->max_cap = 1;
        else if (is_of_type((unitp_t)ulink->data, BOUND_PACK_BINDING)) {
            if (is_lambda4((unitp_t)enc_node->data))
                /* capa = list */
                ((unitp_t)ulink->data)->max_cap = 1;
            else if (is_call((unitp_t)enc_node->data)) {
                /* the name of the rest param has already caused a
                 * decrement of call's max capacity, so we put that 1
                 * capacity back into the capacity of the rest
                 * param. also the call itself will now have no more
                 * capacity as all upcoming units will be absorbed by
                 * the rest param. */
                ((unitp_t)ulink->data)->max_cap = ((unitp_t)enc_node->data)->max_cap + 1;
                ((unitp_t)enc_node->data)->max_cap = 0; /* suddenly, no decrementing! */
            }
        } else if (is_define((unitp_t)ulink->data)) {
            /* capa = name, data */
            ((unitp_t)ulink->data)->max_cap = 2;
        } else if (is_cith((unitp_t)ulink->data)) {
            /* capa = index, pack */
            ((unitp_t)ulink->data)->max_cap = 2;
        } else if (is_tila_show((unitp_t)ulink->data)) {
            /* capa = thing */
            ((unitp_t)ulink->data)->max_cap = 1;
        } else if (is_tila_size((unitp_t)ulink->data))
            /* capa = list */
            ((unitp_t)ulink->data)->max_cap = 1;
        else if (is_tila_nth((unitp_t)ulink->data))
            /* capa = nth, list */
            ((unitp_t)ulink->data)->max_cap = 2;
        else if (is_tila_list((unitp_t)ulink->data))
            /* no capa, Tila_list is used ONLY as default arg to
             * list's &REST:= param! */
            ((unitp_t)ulink->data)->max_cap = 0;
        else if (is_call((unitp_t)ulink->data)) {
            /* capa = the specified number of args to the function */
            int arg_cnt = 0, rpt_cnt = 1; /* args count, repeatitions count */
            digest_call(((unitp_t)ulink->data)->token.str, &arg_cnt, &rpt_cnt);
            ((unitp_t)ulink->data)->max_cap = arg_cnt + 1; /* arg_cnt = args, + 1 = fnc name */
            ((unitp_t)ulink->data)->call_rep_cnt = rpt_cnt;
        } else if (is_cond_if((unitp_t)ulink->data) ||
                   is_cond_then((unitp_t)ulink->data))
            /* capa = expression */
            ((unitp_t)ulink->data)->max_cap = 1;
        else if (is_cond_else((unitp_t)ulink->data)) {
            ((unitp_t)ulink->data)->max_cap = 1;
            ((unitp_t)enc_node->data)->max_cap = 0; /* suddenly, no decrementing! */
        }
        
        if (need_block((unitp_t)ulink->data)) {            
            ((unitp_t)ulink->data)->is_atomic = false;            
            /* new block has it's own environment */
            ((unitp_t)ulink->data)->env = g_hash_table_new(g_str_hash, g_str_equal);
            /* set current binding unit (if it makes bindings bing: boundbinf:= etc.) */
            if (is_lambda4((unitp_t)ulink->data) ||
                is_call((unitp_t)ulink->data) ||
                is_let((unitp_t)ulink->data))
                curr_bind_unit = (unitp_t)ulink->data;            
        } else {			/* an atomic unit */
            ((unitp_t)ulink->data)->is_atomic = true;
            ((unitp_t)ulink->data)->max_cap = 0;
            /* is the unit true or false? set it's boolean type */
            if (is_bool(((unitp_t)ulink->data)))
                ((unitp_t)ulink->data)->type = BOOL;
        }
        
        g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, enc_node->data),
                      -1,
                      g_node_new((unitp_t)ulink->data));

        /* when the unit's enc_node is a lambda/let, the unit is the first item
           of lambda and not a binding form, the unit is taken to be the final
           expression of lambda (i.e. lambda will be closed!) */
        if (((is_lambda4((unitp_t)enc_node->data) ||
              is_let((unitp_t)enc_node->data)) &&
             unit_type((unitp_t)ulink->data) != BINDING &&
             unit_type((unitp_t)ulink->data) != BOUND_BINDING &&
             unit_type((unitp_t)ulink->data) != PACK_BINDING &&
             unit_type((unitp_t)ulink->data) != BOUND_PACK_BINDING)) {
            ((unitp_t)enc_node->data)->max_cap = 0;
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
        unit_type((unitp_t)node->data) != BINDING &&
        unit_type((unitp_t)node->data) != BOUND_BINDING &&
        unit_type((unitp_t)node->data) != PACK_BINDING &&
        unit_type((unitp_t)node->data) != BOUND_PACK_BINDING) {
        fprintf(stderr, "invalid thing in place of parameter\n");
        print_node(node, NULL);
        exit(EXIT_FAILURE);
    }
}

gboolean sanify_lambda(GNode *node, gpointer data) {
    if (is_lambda_node(node)) {
        if (g_node_n_children(node)) {
            GNode *last_child = g_node_last_child(node);
            /* first assert that every child node except with the last one
               is a binding (ie a parameter decleration) */
            g_node_children_foreach(node, G_TRAVERSE_ALL,
                                    (GNodeForeachFunc)assert_func_param,
                                    last_child);
            /* if the last node is a mandatory parameter (i.e. a parameter
               without default value, which i could take out as the lambda
               expression), it is an error! */
            if (((unitp_t)last_child->data)->type == BINDING ||
                unit_type((unitp_t)last_child->data) == PACK_BINDING) {
                fprintf(stderr, "binding '%s' kann nicht ende von deinem lambda sein\n",
                        ((unitp_t)last_child->data)->token.str);
                exit(EXIT_FAILURE);
                /* if the last node LOOKS LIKE a parameter with a default
                   argument (i.e. optional argument/bound binding), we treat
                   it's default argument as the final expression of lambda */
            } else if (unit_type((unitp_t)last_child->data) == BOUND_BINDING ||
                       unit_type((unitp_t)last_child->data) == BOUND_PACK_BINDING) {
                GNode *bound_value = g_node_last_child(last_child);
                g_node_unlink(bound_value);
                g_node_insert(node, -1, bound_value);
                /* and reset the parameter types to their mandatory versions */
                if (unit_type((unitp_t)last_child->data) == BOUND_BINDING)
                    ((unitp_t)last_child->data)->type = BINDING;
                /* if we took the last and only child of the bound pack
                   binding out as the lambda expression and it has no
                   children, change it's type to a simple pack binding without
                   default arguments */
                else if (g_node_n_children(last_child) == 0)
                    ((unitp_t)last_child->data)->type = PACK_BINDING;
            }
        } else {
            fprintf(stderr, "malformed lambda\n");
            print_node(node, NULL);
            fprintf(stderr, "missing expression\n");
            exit(EXIT_FAILURE);
        }
    }
    return false;
}

void post_parse_lambda_check(GNode *root) {
    puts("parse-time lambda sanify");
    g_node_traverse(root, G_PRE_ORDER,
                    G_TRAVERSE_ALL, -1,
                    (GNodeTraverseFunc)sanify_lambda, NULL);
    puts("  lambdas sanified");
}


/* schmelz die 2 zusammen mit der obigen */
void assert_bound_binding_node(GNode *node, GNode *last_child) {
    /* e.g. a parameter or a binding in let */
    if (node != last_child &&
        !is_of_type((unitp_t)node->data, BOUND_BINDING)) {
        fprintf(stderr, "bound binding assertion failed\n");
        print_node(node, NULL);
        fprintf(stderr, "not bound\n");
        exit(EXIT_FAILURE);
    }
}

void
assert_call_param(GNode *binding, GNode *func_node)
{
    if ((binding != func_node && unit_type((unitp_t)binding->data) == BINDING)) {
        fprintf(stderr, "malformed argument passed\n");
        print_node(binding, NULL);
        fprintf(stderr, "is not bound\n");
        exit(EXIT_FAILURE);
    }
}

gboolean check_pass(GNode *node, gpointer data) {
    if (is_call((unitp_t)node->data)) {
        if (g_node_n_children(node)) {
            /* GNode *func_node = g_node_last_child(node); */
            GNode *func_node = g_node_nth_child(node, 0);
            /* if (unit_type((unitp_t)func_node->data) != NAME) { */
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
        if (is_of_type((unitp_t)node->data, BINDING) ||
            is_of_type((unitp_t)node->data, BOUND_BINDING)) {
            fprintf(stderr, "let final expression cant be a binding\n");
            print_node(node, NULL);
            exit(EXIT_FAILURE);
        }
    } else {                    /* bindings */
        if (!is_of_type((unitp_t)node->data, BOUND_BINDING)) {
            fprintf(stderr, "let binding not bound\n");
            print_node(node, NULL);
            exit(EXIT_FAILURE);                        
        }
    }
}

gboolean check_let(GNode *node, gpointer data) {
    if (is_let((unitp_t)node->data)) {
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
