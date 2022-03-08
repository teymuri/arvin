#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <math.h>
#include "type.h"
#include "token.h"
#include "unit.h"
#include "ast.h"
#include "print.h"

void eval_tila_list(struct Tila_data **result, GNode *node, GHashTable *env);
void eval_call(struct Tila_data **, GNode *, GHashTable *);
void eval_cpack(struct Tila_data **, GNode *, GHashTable *, guint, guint);

char *binding_node_name(GNode *node) {
    struct Unit *u = ((unitp_t)node->data);
    char *s = u->token.str;
    char *name;
    if (is_of_type(u, BINDING)) {
        name = (char *)malloc(strlen(s));
        strncpy(name, s, strlen(s) - 1);
        name[strlen(s) - 1] = '\0';
    }
    else if (is_of_type(u, BOUND_BINDING)) {
        name = (char *)malloc(strlen(s)-1);
        strncpy(name, s, strlen(s) - 2);
        name[strlen(s) - 2] = '\0';
    }
    else if (is_of_type(u, PACK_BINDING)) {
        name = (char *)malloc(strlen(s)-1);
        strncpy(name, s+1, strlen(s) - 2);
        name[strlen(s) - 2] = '\0';
    } else if (is_of_type(u, BOUND_PACK_BINDING)) {
        name = (char *)malloc(strlen(s)-2);
        strncpy(name, s+1, strlen(s) - 3);
        name[strlen(s) - 3] = '\0';
    }
    return name;
}

void put_hash_table_entry(gpointer key, gpointer val, gpointer ht) {
    g_hash_table_insert(ht, key, val);
}

GHashTable *clone_hash_table(GHashTable *ht) {
    GHashTable *new = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_foreach(ht, (GHFunc)put_hash_table_entry, new);
    return new;
}

struct Tila_data *eval3(GNode *, GHashTable *);


void
eval_tila_show(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct Tila_data *d = eval3(node->children, env);
    set_data(*result, d);
    switch (d->type) {
    case INTEGER:
        printf("%d\n", d->slots.tila_int);
        break;
    case FLOAT:
        printf("%f\n", d->slots.tila_float);
        break;
    case BOOL:
        printf("%s\n", d->slots.tila_bool ? TRUE_KW : FALSE_KW);
        break;
    case LAMBDA:
        printf("tbi:lambda (to be implemented)\n");
        break;
    default:
        printf("show received unknown data type %d\n", d->type);
        break;
    }
}

gint get_param_index(GList *list, char *str) {
  bool found = false;
  gint idx = 0;
  while (list) {
    if (!strcmp(((unitp_t)list->data)->token.str, str)) {
      found = true;
      break;      
    }
    list = list->next;
    idx++;
  }
  if (found) return idx;
  else return -1;
}

void
eval_cond(struct Tila_data **result, GNode *node, GHashTable *env)
{
    guint clause_count = g_node_n_children(node);
    struct Tila_data *d;
    guint i = 0;
    for (; i < clause_count - 1; i += 2) {
        if (eval3(g_node_nth_child(node, i)->children, env)->slots.tila_bool) {
            /* eval the corresponding Then clause and quit */
            d = eval3(g_node_nth_child(node, i + 1)->children, env);
            set_data(*result, d);
            break;
        }
    }
    /* else eval the last (Else) clause ONLY if traveled
     * unsuccessfully through every clause, but not if landed here
     * because of the above break! */
    if (i == clause_count - 1) {
        d = eval3(g_node_nth_child(node, clause_count - 1)->children, env);
        set_data(*result, d);        
    }
}

/* ******** hof begin ******** */

guint
min_sublist_size(struct List list)
{
    guint lst_len;
    guint min = G_MAXUINT;
    while (list.item) {
        lst_len = ((struct Tila_data *)list.item->data)->slots.tila_list->size;
        if (lst_len < min)
            min = lst_len;
        list.item = list.item->next;
    }
    return min;
}

void
pack_evaled_list(struct Tila_data **result, GNode *node)
{
    (*result)->type = LIST;
    struct List *list = (struct List *)malloc(sizeof (struct List));
    list->item = NULL;
    list->size = 0;
    while (node) {
        list->item = g_list_append(list->item, (struct Tila_data *)node->data);
        list->size++;
        node = node->next;      /* node's sibling */
    }
    (*result)->slots.tila_list = list;
}

void
eval_tila_lfold(struct Tila_data **result, GNode *node, GHashTable *env)
{
    /* list is a list of lists */
    struct List *list = eval3(g_node_nth_child(node, 0), env)->slots.tila_list;
    GList *list_item_cp = list->item;
    struct Lambda *lambda = eval3(g_node_nth_child(node, 1), env)->slots.tila_lambda;
    struct Tila_data *acc = eval3(g_node_nth_child(node, 2), env);
    GHashTable *call_env = clone_hash_table(lambda->env);
    guint minsz = min_sublist_size(*list);
    guint param_idx = 0;
    while (minsz--) {
        while (param_idx < g_list_length(lambda->param_list)) {
            if (unit_type((unitp_t)g_node_nth_child(lambda->node, param_idx)->data) == PACK_BINDING ||
                unit_type((unitp_t)g_node_nth_child(lambda->node, param_idx)->data) == BOUND_PACK_BINDING) {
                GNode *args_node = g_node_new(NULL);
                g_node_insert(args_node, -1,
                              g_node_new(param_idx ? ((struct Tila_data *)list->item->data)->slots.tila_list->item->data : acc));
                if (param_idx) {
                    ((struct Tila_data *)list->item->data)->slots.tila_list->item = ((struct Tila_data *)list->item->data)->slots.tila_list->item->next;
                    list->item = list->item->next;
                }
                while (list->item) {
                    g_node_insert(args_node, -1, g_node_new(((struct Tila_data *)list->item->data)->slots.tila_list->item->data));
                    ((struct Tila_data *)list->item->data)->slots.tila_list->item = ((struct Tila_data *)list->item->data)->slots.tila_list->item->next;
                    list->item = list->item->next;
                }
                struct Tila_data *rest_args = malloc(sizeof (struct Tila_data));
                pack_evaled_list(&rest_args, args_node->children);
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda->node, param_idx)),
                                    rest_args);
                g_node_destroy(args_node);
                break;
            } else {
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda->node, param_idx)),
                                    param_idx ? ((struct Tila_data *)list->item->data)->slots.tila_list->item->data : acc);
                if (param_idx) {
                    ((struct Tila_data *)list->item->data)->slots.tila_list->item = ((struct Tila_data *)list->item->data)->slots.tila_list->item->next;
                    list->item = list->item->next;
                }
            }
            param_idx++;
        }
        acc = eval3(g_node_last_child(lambda->node), call_env);
        list->item = list_item_cp;
        param_idx = 0;
    }
    set_data(*result, acc);
}


/* ******** hof end ******** */

/* ******** math begin ******** */
void
eval_tila_expt(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct Tila_data *base = eval3(g_node_nth_child(node, 0), env);
    struct Tila_data *expt = eval3(g_node_nth_child(node, 1), env);
    if (base->type == INTEGER && expt->type == INTEGER) {
        (*result)->type = INTEGER;
        (*result)->slots.tila_int = (int)powf(base->slots.tila_int, expt->slots.tila_int);
    } else {
        (*result)->type = FLOAT;
        if (base->type == FLOAT && expt->type == INTEGER)
            (*result)->slots.tila_float = base->slots.tila_float + expt->slots.tila_int;
        else if (base->type == INTEGER && expt->type == FLOAT)
            (*result)->slots.tila_float = base->slots.tila_int + expt->slots.tila_float;
        else if (base->type == FLOAT && expt->type == FLOAT)
            (*result)->slots.tila_float = base->slots.tila_float + expt->slots.tila_float;
        else {
            fprintf(stderr, "non-num to add\n");
            exit(EXIT_FAILURE);
        }
    }
}

void
eval_tila_add(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.tila_list;
    struct Tila_data *d;
    float acc = 0;
    bool allints = true;
    while (lst->item) {
        d = lst->item->data;
        if (d->type == FLOAT) {
            acc += d->slots.tila_float;
            allints = false;
        } else if (d->type == INTEGER)
            acc += d->slots.tila_int;
        else {
            fprintf(stderr, "non-num to add\n");
            exit(EXIT_FAILURE);            
        }
        lst->item = lst->item->next;
    }
    if (allints) {
        (*result)->type = INTEGER;
        (*result)->slots.tila_int = (int)acc;
    }
    else {
        (*result)->type = FLOAT;
        (*result)->slots.tila_float = acc;
    }
}

void
eval_mul_op(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.tila_list;
    struct Tila_data *d;
    float acc = 1;
    bool allints = true;
    while (lst->item) {
        d = lst->item->data;
        if (d->type == FLOAT) {
            acc *= d->slots.tila_float;
            allints = false;
        } else if (d->type == INTEGER)
            acc *= d->slots.tila_int;
        else {
            fprintf(stderr, "non-num to add\n");
            exit(EXIT_FAILURE);            
        }
        lst->item = lst->item->next;
    }
    if (allints) {
        (*result)->type = INTEGER;
        (*result)->slots.tila_int = (int)acc;
    }
    else {
        (*result)->type = FLOAT;
        (*result)->slots.tila_float = acc;
    }
}

void
eval_sub_op(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.tila_list;
    if (lst->size) {
        bool allints = true;
        float acc;
        struct Tila_data *d = lst->item->data;
        if (d->type == INTEGER) {
            acc = (float)d->slots.tila_int;
        } else if (d->type == FLOAT) {
            allints = false;
            acc = d->slots.tila_float;
        }
        lst->item = lst->item->next;
        while (lst->item) {
            d = lst->item->data;
            if (d->type == FLOAT) {
                acc -= d->slots.tila_float;
                allints = false;
            } else if (d->type == INTEGER)
                acc -= d->slots.tila_int;
            else {
                fprintf(stderr, "non-num to add\n");
                exit(EXIT_FAILURE);            
            }
            lst->item = lst->item->next;
        }
        if (allints) {
            (*result)->type = INTEGER;
            (*result)->slots.tila_int = (int)acc;
        }
        else {
            (*result)->type = FLOAT;
            (*result)->slots.tila_float = acc;
        }        
    } else {
        (*result)->type = INTEGER;
        (*result)->slots.tila_int = 0;
    }
}
void
eval_div_op(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.tila_list;
    if (lst->size) {
        bool allints = true;
        float acc;
        struct Tila_data *d = lst->item->data;
        if (d->type == INTEGER) {
            acc = (float)d->slots.tila_int;
        } else if (d->type == FLOAT) {
            allints = false;
            acc = d->slots.tila_float;
        }
        lst->item = lst->item->next;
        while (lst->item) {
            d = lst->item->data;
            if (d->type == FLOAT) {
                acc /= d->slots.tila_float;
                allints = false;
            } else if (d->type == INTEGER)
                acc /= d->slots.tila_int;
            else {
                fprintf(stderr, "non-num to add\n");
                exit(EXIT_FAILURE);            
            }
            lst->item = lst->item->next;
        }
        if (allints) {
            (*result)->type = INTEGER;
            (*result)->slots.tila_int = (int)acc;
        }
        else {
            (*result)->type = FLOAT;
            (*result)->slots.tila_float = acc;
        }        
    } else {
        (*result)->type = INTEGER;
        (*result)->slots.tila_int = 1;
    }
}





/* ******** math end ******** */
/* ******* begin list ******* */
void
eval_tila_list(struct Tila_data **result, GNode *node, GHashTable *env)
/* node is the first item to the list */
{
    (*result)->type = LIST;
    struct List *list = (struct List *)malloc(sizeof (struct List));
    list->item = NULL;
    list->size = 0;
    while (node) {
        /* each item of a list is evaluated at list creation-time */
        list->item = g_list_append(list->item, eval3(node, env));
        list->size++;
        node = node->next;      /* node's sibling */
    }
    (*result)->slots.tila_list = list;
}

void
eval_tila_nth(struct Tila_data **result, GNode *node, GHashTable *env)
{
    guint idx = eval3(g_node_nth_child(node, 0), env)->slots.tila_int;
    struct List *list = eval3(g_node_nth_child(node, 1), env)->slots.tila_list;
    struct Tila_data *data = g_list_nth(list->item, idx)->data; /* item = first link of list */
    set_data(*result, data);
}

void eval_tila_size(struct Tila_data **result, GNode *node, GHashTable *env)
{
    struct Tila_data *d = eval3(node->children, env);
    /* work only if data is list */
    if (d->type == LIST) {
        (*result)->type = INTEGER;
        (*result)->slots.tila_int = d->slots.tila_list->size;
    } else {
        fprintf(stderr, "size arg must eval to list\n");
        print_node(node->children, NULL);
        exit(EXIT_FAILURE);   
    }
}

/* ***** end list ******* */


void
eval_lambda(struct Tila_data **result, GNode *node, GHashTable *env)
{
    (*result)->type = LAMBDA;
    struct Lambda *lambda = malloc(sizeof (struct Lambda));
    /* make a snapshot of the current environment at the time of
     * lambda creation */
    lambda->env = clone_hash_table(env);
    lambda->node = node;
    lambda->param_list = NULL;
    /* add parameters to env */
    for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
        GNode *binding = g_node_nth_child(node, i);
        char *name = binding_node_name(binding);
        lambda->param_list = g_list_append(lambda->param_list, name);
        /* if it is an optional parameter save it's value */
        if (unit_type((unitp_t)binding->data) == BOUND_BINDING ||
            /* default arg to a rest param is a single list */
            unit_type((unitp_t)binding->data) == BOUND_PACK_BINDING) {
            g_hash_table_insert(lambda->env, name, eval3(binding->children, lambda->env));
        } else if (unit_type((unitp_t)binding->data) == BINDING ||
                   unit_type((unitp_t)binding->data) == PACK_BINDING) {
            g_hash_table_insert(lambda->env, name, NULL);
        }
    }
    (*result)->slots.tila_lambda = lambda;
}



/* pack contains Tila_data pointers */
/* pack start upto end children, when end == 0 packt alle kinder bis zum ende */
void eval_cpack(struct Tila_data **result, GNode *node,
                GHashTable *env, guint start, guint end) {
    guint n = end ? end : g_node_n_children(node);
    GList *pack = NULL;
    for (guint i = start; i < n; i++)
        pack = g_list_append(pack, eval3(g_node_nth_child(node, i), env));
    (*result)->type = PACK;
    (*result)->slots.pack = pack;
}


void eval_named_rest_args(struct Tila_data **result, GNode *node, GHashTable *env)
{
    GList *pack = NULL;
    guint count = g_node_n_children(node);
    for (guint n = 0; n < count; n++)
        pack = g_list_append(pack, eval3(g_node_nth_child(node, n), env));
    (*result)->type = PACK;
    (*result)->slots.pack = pack;
}
void eval_unnamed_rest_args(struct Tila_data **result, GNode *node, GHashTable *env)
{
    GList *pack = NULL;
    while (node) {
        pack = g_list_append(pack, eval3(node, env));
        node = node->next;      /* node's sibling */
    }
    (*result)->type = PACK;
    (*result)->slots.pack = pack;
}

void eval_let(struct Tila_data **result, GNode *node, GHashTable *env) {
    ((unitp_t)node->data)->env = clone_hash_table(env);
    for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
        GNode *binding = g_node_nth_child(node, i);
        char *name = binding_node_name(binding);
        g_hash_table_insert(((unitp_t)node->data)->env, name,
                            eval3(binding->children, ((unitp_t)node->data)->env));
    }
    (*result) = eval3(g_node_last_child(node), ((unitp_t)node->data)->env);
}

void eval_define(struct Tila_data **result, GNode *node, GHashTable *env) {
    char *name = ((unitp_t)g_node_nth_child(node, 0)->data)->token.str;
    struct Tila_data *data = eval3(g_node_nth_child(node, 1), env);
    /* definitions are always saved in the global environment, no
       matter in which environment we are currently */
    g_hash_table_insert(((unitp_t)g_node_get_root(node)->data)->env, name, data);
    set_data(*result, data);
}

gint find_param_idx(GList *param_lst_lnk, char *str) {
    bool found = false;
    gint idx = 0;
    while (param_lst_lnk) {
        if (!strcmp(param_lst_lnk->data, str)) {
            found = true;
            break;      
        }
        param_lst_lnk = param_lst_lnk->next;
        idx++;
    }
    if (found) return idx;
    else return -1;
}

GNode *
nth_sibling(GNode *node, int n)
{
    while (n-- && node->next)
        node = node->next;
    return node;
}

void
eval_call(struct Tila_data **result, GNode *node, GHashTable *env)
{
    GNode *lambda_node = g_node_first_child(node);
    struct Tila_data *lambda_data = eval3(lambda_node, env);
    /* make a copy of the lambda env just for this call */
    GHashTable *call_env = clone_hash_table(lambda_data->slots.tila_lambda->env);
    guint arg_idx = 0;
    gint param_idx = 0;
    GNode *first_arg = g_node_nth_child(node, 1);
    /* iterate over passed arguments */
    while (arg_idx < g_node_n_children(node) - 1) {
        if (unit_type((unitp_t)nth_sibling(first_arg, arg_idx)->data) == BOUND_BINDING) {
            g_hash_table_insert(call_env,
                                binding_node_name(nth_sibling(first_arg, arg_idx)),
                                eval3(nth_sibling(first_arg, arg_idx)->children, env));
            
            param_idx = find_param_idx(lambda_data->slots.tila_lambda->param_list,
                                         binding_node_name(nth_sibling(first_arg, arg_idx)));

            if (param_idx == -1) {
                fprintf(stderr, "unknown parameter\n");
                print_node(nth_sibling(first_arg, arg_idx), NULL);
                fprintf(stderr, "passed to\n");
                print_node(lambda_data->slots.tila_lambda->node, NULL);
                exit(EXIT_FAILURE);
            } else {
                arg_idx++;
                param_idx++;
            }
        } else if (is_of_type((unitp_t)nth_sibling(first_arg, arg_idx)->data, BOUND_PACK_BINDING)) {
            struct Tila_data *rest_args = malloc(sizeof (struct Tila_data));
            /* named rest args */
            /* eval the passed arguments in the  */
            eval_tila_list(&rest_args, nth_sibling(first_arg, arg_idx)->children, env);
            g_hash_table_insert(call_env,
                                binding_node_name(nth_sibling(first_arg, arg_idx)),
                                rest_args);
            break;       /* out of parameter processing, &rest: must be the last! */
        } else {			/* just an expression or multiple expressions, not a binding (para->arg) */
            if (unit_type((unitp_t)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data) == PACK_BINDING ||
                unit_type((unitp_t)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data) == BOUND_PACK_BINDING) {
                /* reached rest args without param name */
                struct Tila_data *rest_args = malloc(sizeof (struct Tila_data));
                eval_tila_list(&rest_args, nth_sibling(first_arg, arg_idx), env);
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)),
                                    rest_args);
                break;                  /* last param, get out!!! */
            } else {
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)),
                                    eval3(nth_sibling(first_arg, arg_idx), env));
                arg_idx++;
                param_idx++;
            }
        }
    }
    /* eval the lambda expr in the created call-time env */
    *result = eval3(g_node_last_child(lambda_data->slots.tila_lambda->node), call_env);
}

void eval_toplvl(struct Tila_data **result, GNode *root)
{
    guint size = g_node_n_children(root);
    for (guint i = 0; i < size - 1; i++)
        /* every other toplevel part of code inherits the environment of the toplevel */
        eval3(g_node_nth_child(root, i), ((unitp_t)root->data)->env);
    *result = eval3(g_node_nth_child(root, size - 1), ((unitp_t)root->data)->env);
}

struct Tila_data *
eval3(GNode *node, GHashTable *env)
{
    struct Tila_data *result = malloc(sizeof (struct Tila_data));
    if (((unitp_t)node->data)->is_atomic) {
        switch (unit_type(((unitp_t)node->data))) {
        case INTEGER:
            result->type = INTEGER;
            result->slots.tila_int = ((unitp_t)node->data)->ival;
            break;
        case FLOAT:
            result->type = FLOAT;
            result->slots.tila_float = ((unitp_t)node->data)->fval;
            break;
        case BOOL:              /* a literal boolean (i.e. true/false) */
            result->type = BOOL;
            result->slots.tila_bool = is_true((unitp_t)node->data) ? true : false;
            break;
        case NAME:
        {
            char *wanted_kw = ((unitp_t)node->data)->token.str;
            /* symbols are evaluated in the envs of their enclosing units */
            struct Tila_data *tdata;
            GNode *nodecp = node; /* copy node for print_node belowv */
            do {
                if ((tdata = g_hash_table_lookup(env, wanted_kw))) {
                    set_data(result, tdata);
                    break;
                }
                /* the && env = ... here is just for the purpose of
                 * the assignment itself (supposing the node
                 * assignment and check part happened to be
                 * successful) and not really a condition check */
            } while ((node = node->parent) && (env = ((unitp_t)node->data)->env));
            if (!node) {
                fprintf(stderr, "lookup failed for\n");
                print_node(nodecp, NULL);
                exit(EXIT_FAILURE);
            }
        } break;                  /* break the NAME branch */
        default: break;		/* undefined unit type */
        }
    } else {			/* builtin stuff */
        if ((((unitp_t)node->data)->uuid == 0)) {
            eval_toplvl(&result, node); /* toplevel uses it's own environment */
        } else if (is_tila_show((unitp_t)node->data))
            eval_tila_show(&result, node, env);
        else if (is_lambda4((unitp_t)node->data)) {
            eval_lambda(&result, node, env);
        } else if (is_define((unitp_t)node->data)) { /* define */
            eval_define(&result, node, env);
        } else if (is_let((unitp_t)node->data)) {
            eval_let(&result, node, env);
        } else if (is_cpack((unitp_t)node->data)) {
            eval_cpack(&result, node, env, 0, 0);
        }  else if (is_call((unitp_t)node->data)) {
            for (int i = 0; i < ((unitp_t)node->data)->call_rpt_cnt; i++)
                eval_call(&result, node, env);
        }            
        else if (is_tila_nth((unitp_t)node->data))
            eval_tila_nth(&result, node, env);
        else if (is_tila_size((unitp_t)node->data))
            eval_tila_size(&result, node, env);
        else if (is_tila_list((unitp_t)node->data))
            /* List is used in the core ONLY as default argument the
             * &ITEMS:= param of the list function to get an empty
             * list. although invoking it would simply result in an
             * empty list (as it's max capacity is 0 it cant take any
             * args) try NOT to use it elsewhere as it's internal and
             * can change without notice! */
            eval_tila_list(&result, node->children, env);
        else if (is_cond((unitp_t)node->data))
            eval_cond(&result, node, env);
        else if (is_tila_add((unitp_t)node->data))
            eval_tila_add(&result, node, env);
        else if (is_sub((unitp_t)node->data))
            eval_sub_op(&result, node, env);
        else if (is_mul((unitp_t)node->data))
            eval_mul_op(&result, node, env);
        else if (is_div_op((unitp_t)node->data))
            eval_div_op(&result, node, env);
        else if (is_tila_expt((unitp_t)node->data))
            eval_tila_expt(&result, node, env);
        else if (is_tila_fold((unitp_t)node->data))
            eval_tila_lfold(&result, node, env);
    }
    return result;
}
