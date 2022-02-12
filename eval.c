#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include "type.h"
#include "token.h"
#include "unit.h"
#include "core.h"
#include "ast.h"
#include "print.h"


void eval_cpack(struct Tila_data **, GNode *, GHashTable *, guint, guint);


char *binding_node_name(GNode *node) {
    struct Unit *u = ((unitp_t)node->data);
    char *s = u->token.str;
    char *name;
    if (is_of_type(u, BINDING) || is_of_type(u, BOUND_BINDING)) {
        name = (char *)malloc(strlen(s));
        strncpy(name, s, strlen(s) - 1);
        name[strlen(s) - 1] = '\0';
    } else if (is_of_type(u, PACK_BINDING) || is_of_type(u, BOUND_PACK_BINDING)) {
        name = (char *)malloc(strlen(s)-1);
        strncpy(name, s+1, strlen(s) - 2);
        name[strlen(s) - 2] = '\0';
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

void eval_pret(struct Tila_data **result, GNode *root, GHashTable *env) {
    *result = pret(eval3(root->children, env));
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

void eval_lambda(struct Tila_data **result, GNode *node, GHashTable *env) {
    (*result)->type = LAMBDA;
    struct Lambda *lambda = malloc(sizeof (struct Lambda));
    lambda->env = clone_hash_table(env);
    lambda->node = node;
    lambda->param_list = NULL;
    lambda->arity = ((unitp_t)node->data)->arity;
    /* add parameters to env */
    for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
        GNode *binding = g_node_nth_child(node, i);
        char *name = binding_node_name(binding);
        lambda->param_list = g_list_append(lambda->param_list, (unitp_t)binding->data);
        /* if it is an optional parameter save it's value */
        if (unit_type((unitp_t)binding->data) == BOUND_BINDING) {
            g_hash_table_insert(lambda->env, name, eval3(binding->children, lambda->env));
        } else if (unit_type((unitp_t)binding->data) == BINDING ||
                   unit_type((unitp_t)binding->data) == PACK_BINDING) {
            g_hash_table_insert(lambda->env, name, NULL);
        } else if (unit_type((unitp_t)binding->data) == BOUND_PACK_BINDING) {
            struct Tila_data *rest_params = malloc(sizeof (struct Tila_data));
            /* 0tes Kind bis zum letzten Kind (mit allen seinen siblings) */
            eval_cpack(&rest_params, binding, lambda->env, 0, 0);
            g_hash_table_insert(lambda->env, name, rest_params);
        }
    }
    (*result)->data.slot_lambda = lambda;
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
    (*result)->data.pack = pack;
}


void eval_named_rest_args(struct Tila_data **result, GNode *node, GHashTable *env)
{
    GList *pack = NULL;
    guint count = g_node_n_children(node);
    for (guint n = 0; n < count; n++)
        pack = g_list_append(pack, eval3(g_node_nth_child(node, n), env));
    (*result)->type = PACK;
    (*result)->data.pack = pack;
}
void eval_unnamed_rest_args(struct Tila_data **result, GNode *node, GHashTable *env)
{
    GList *pack = NULL;
    while (node) {
        pack = g_list_append(pack, eval3(node, env));
        node = node->next;      /* node's sibling */
    }
    (*result)->type = PACK;
    (*result)->data.pack = pack;
}

void eval_tila_list(struct Tila_data **tdata, GNode *node, GHashTable *env)
{
    struct List *list = (struct List *)malloc(sizeof (struct List));
    list->item = NULL;
    list->size = g_node_n_children(node->parent);
    while (node) {
        list->item = g_list_append(list->item, eval3(node, env));
        node = node->next;      /* node's sibling */
    }
    (*tdata)->type = LIST;
    (*tdata)->data.list = list;
}

void eval_tila_nth(struct Tila_data **result, GNode *node, GHashTable *env) {
    struct Tila_data *idx_data = eval3(g_node_nth_child(node, 0), env);
    struct Tila_data *pack_data = eval3(g_node_nth_child(node, 1), env);
    guint idx = (guint)idx_data->data.int_slot;
    GList *pack = pack_data->data.list->item;
    struct Tila_data *data = g_list_nth(pack, idx)->data;
    (*result)->type = data->type;
    set_data_slot(*result, data);
}


void eval_cith(struct Tila_data **result, GNode *node, GHashTable *env) {
    struct Tila_data *idx_data = eval3(g_node_nth_child(node, 0), env);
    struct Tila_data *pack_data = eval3(g_node_nth_child(node, 1), env);
    guint idx = (guint)idx_data->data.int_slot;
    GList *pack = pack_data->data.pack;
    struct Tila_data *data = g_list_nth(pack, idx)->data;
    (*result)->type = data->type;
    set_data_slot(*result, data);
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
    (*result)->type = data->type;
    set_data_slot(*result, data);
}

gint lambda_param_idx(GList *list, char *str) {
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

GNode *nth_sibling(GNode *node, int n)
{
    while (n-- && node->next)
        node = node->next;
    return node;
}

void eval_call(struct Tila_data **result, GNode *node, GHashTable *env)
{
    GNode *lambda_node = g_node_first_child(node);
    /* populate lambda environment */
    struct Tila_data *lambda_data = eval3(lambda_node, env);
    /* make a copy of the lambda env */
    GHashTable *call_time_env = clone_hash_table(lambda_data->data.slot_lambda->env);
    /* iterate over passed arguments */
    guint arg_idx = 0;
    gint param_idx = 0;
    GNode *first_arg = g_node_nth_child(node, 1);
    
    while (arg_idx < g_node_n_children(node) - 1) {
        if (unit_type((unitp_t)nth_sibling(first_arg, arg_idx)->data) == BOUND_BINDING) {
            g_hash_table_insert(call_time_env,
                                binding_node_name(nth_sibling(first_arg, arg_idx)),
                                eval3(nth_sibling(first_arg, arg_idx)->children, env)); /* nicht call_time env???? */
            
            param_idx = lambda_param_idx(lambda_data->data.slot_lambda->param_list,
                                         ((unitp_t)nth_sibling(first_arg, arg_idx)->data)->token.str);

            if (param_idx == -1) {
                fprintf(stderr, "unknown parameter\n");
                print_node(nth_sibling(first_arg, arg_idx), NULL);
                fprintf(stderr, "passed to\n");
                print_node(lambda_data->data.slot_lambda->node, NULL);
                exit(EXIT_FAILURE);
            } else {
                arg_idx++;
                param_idx++;
            }
        } else if (is_of_type((unitp_t)nth_sibling(first_arg, arg_idx)->data, BOUND_PACK_BINDING)) {
            struct Tila_data *rest_params = malloc(sizeof (struct Tila_data));
            /* is named rest args */
            eval_tila_list(&rest_params, nth_sibling(first_arg, arg_idx)->children, call_time_env);
            /* eval_named_rest_args(&rest_params, nth_sibling(first_arg, arg_idx), call_time_env); */
            g_hash_table_insert(call_time_env,
                                binding_node_name(nth_sibling(first_arg, arg_idx)),
                                rest_params);
            break;       /* out of parameter processing, &rest: must be the last! */
        } else {			/* just an expression or multiple expressions, not a binding (para->arg) */
            if (unit_type((unitp_t)g_node_nth_child(lambda_data->data.slot_lambda->node, param_idx)->data) == PACK_BINDING ||
                unit_type((unitp_t)g_node_nth_child(lambda_data->data.slot_lambda->node, param_idx)->data) == BOUND_PACK_BINDING) {
                /* reached rest args without param name */
                struct Tila_data *rest_params = malloc(sizeof (struct Tila_data));
                eval_tila_list(&rest_params, nth_sibling(first_arg, arg_idx), call_time_env);
                /* eval_unnamed_rest_args(&rest_params, nth_sibling(first_arg, arg_idx), call_time_env); */
                g_hash_table_insert(call_time_env,
                                    binding_node_name(g_node_nth_child(lambda_data->data.slot_lambda->node, param_idx)),
                                    rest_params);
                break;                  /* last param, get out!!! */
            } else {
                g_hash_table_insert(call_time_env,
                                    binding_node_name(g_node_nth_child(lambda_data->data.slot_lambda->node, param_idx)),
                                    eval3(nth_sibling(first_arg, arg_idx), env));
                arg_idx++;
                param_idx++;
            }
        }
    }
    *result = eval3(g_node_last_child(lambda_data->data.slot_lambda->node), call_time_env);
}

void eval_pass(struct Tila_data **result, GNode *node, GHashTable *env) {
    GNode *lambda_node = g_node_last_child(node);
    /* populate lambda environment */
    struct Tila_data *x = eval3(lambda_node, env);
    /* make a copy of the lambda env */
    GHashTable *call_time_env = clone_hash_table(x->data.slot_lambda->env);
    /* iterate over passed arguments */
    gint idx = 0;			/* gint because of g_node_child_index update later */
    while (idx < (gint)g_node_n_children(node) - 1) {
        if (unit_type((unitp_t)g_node_nth_child(node, idx)->data) == BOUND_BINDING) {
            g_hash_table_insert(call_time_env,
                                binding_node_name(g_node_nth_child(node, idx)),
                                eval3(g_node_nth_child(node, idx)->children, env));
            /* update the index to reflect the position of current passed
               argument in the parameter list of the lambda */
            gint in_lambda_idx = lambda_param_idx(x->data.slot_lambda->param_list,
                                                  ((unitp_t)g_node_nth_child(node, idx)->data)->token.str);;
            if (in_lambda_idx == -1) {
                fprintf(stderr, "unknown parameter\n");
                print_node(g_node_nth_child(node, idx), NULL);
                fprintf(stderr, "passed to\n");
                print_node(x->data.slot_lambda->node, NULL);
                exit(EXIT_FAILURE);
            } else if (in_lambda_idx > idx) {
                if (unit_type((unitp_t)g_node_nth_child(node, idx+1)->data) == BOUND_BINDING) {
                    idx++;
                } else idx = in_lambda_idx;
            } else idx++;
        } else if (unit_type((unitp_t)g_node_nth_child(node, idx)->data) == BOUND_PACK_BINDING) {
            struct Tila_data *rest_params = malloc(sizeof (struct Tila_data));
            eval_cpack(&rest_params,
                            g_node_nth_child(node, idx),
                            call_time_env, 0, 0);
            g_hash_table_insert(call_time_env,
                                binding_node_name(g_node_nth_child(node, idx)),
                                rest_params);
            break;       /* out of parameter processing, &rest: must be the last! */
        } else {			/* just an expression or multiple expressions, not a binding (para->arg) */
            if (unit_type((unitp_t)g_node_nth_child(x->data.slot_lambda->node, idx)->data) == PACK_BINDING ||
                unit_type((unitp_t)g_node_nth_child(x->data.slot_lambda->node, idx)->data) == BOUND_PACK_BINDING) {
                /* es ist pack binding ohne keyword */
                struct Tila_data *rest_params = malloc(sizeof (struct Tila_data));
                eval_cpack(&rest_params,
                                node,
                                call_time_env,
                                idx,
                                g_node_n_children(node) - 1);
                g_hash_table_insert(call_time_env,
                                    binding_node_name(g_node_nth_child(x->data.slot_lambda->node, idx)),
                                    rest_params);
                break;                  /* last param, get out!!! */
            } else {
                char *bname = binding_node_name(g_node_nth_child(x->data.slot_lambda->node, idx));
                g_hash_table_insert(call_time_env, bname,
                                    eval3(g_node_nth_child(node, idx), env));
                idx++;        
            }
        }
    }
    *result = eval3(g_node_last_child(x->data.slot_lambda->node), call_time_env);
}



void eval_toplevel(struct Tila_data **result, GNode *root)
{
    guint size = g_node_n_children(root);
    for (guint i = 0; i < size - 1; i++)
        /* every other toplevel part of code inherits the environment of the toplevel */
        eval3(g_node_nth_child(root, i), ((unitp_t)root->data)->env);
    *result = eval3(g_node_nth_child(root, size - 1), ((unitp_t)root->data)->env);
}

struct Tila_data *eval3(GNode *node, GHashTable *env)
{
    struct Tila_data *result = malloc(sizeof (struct Tila_data));
    if (((unitp_t)node->data)->is_atomic) {
        switch (unit_type(((unitp_t)node->data))) {
        case INTEGER:
            result->type = INTEGER;
            result->data.int_slot = ((unitp_t)node->data)->ival;
            break;
        case FLOAT:
            result->type = FLOAT;
            result->data.float_slot = ((unitp_t)node->data)->fval;
            break;
        case BOOL:              /* a literal boolean (i.e. true/false) */
            result->type = BOOL;
            result->data.slot_bool = is_true((unitp_t)node->data) ? true : false;
            break;
        case NAME:
        {	  
            char *tokstr = ((unitp_t)node->data)->token.str;
            /* symbols are evaluated in the envs of their enclosing units */
            struct Tila_data *data;
            if ((data = g_hash_table_lookup(env, tokstr))) {
                result->type = data->type;
                set_data_slot(result, data);
            } else {
                GNode *parent = node->parent;
                while (parent) {		  
                    if ((data = g_hash_table_lookup(((unitp_t)parent->data)->env, tokstr))) {
                        result->type = data->type;
                        set_data_slot(result, data);
                        break;  /* out of while */
                    } else {
                        parent = parent->parent;
                    }
                }
                /* nothing found */
                fprintf(stderr, "lookup failed for\n");
                print_node(node, NULL);
                exit(EXIT_FAILURE);
            }
        }
        break;                  /* break the NAME branch */
        default: break;		/* undefined unit type */
        }
    } else {			/* builtin stuff */
        if ((((unitp_t)node->data)->uuid == 0)) {
            eval_toplevel(&result, node); /* toplevel uses it's own environment */
        } else if (is_pret4((unitp_t)node->data)) {
            eval_pret(&result, node, env);
        } else if (is_lambda4((unitp_t)node->data)) {
            eval_lambda(&result, node, env);
        } else if (is_assignment4((unitp_t)node->data)) { /* define */
            eval_define(&result, node, env);
        } else if (is_pass((unitp_t)node->data)) {
            eval_pass(&result, node, env);
        } else if (is_let((unitp_t)node->data)) {
            eval_let(&result, node, env);
        } else if (is_cpack((unitp_t)node->data)) {
            eval_cpack(&result, node, env, 0, 0);
        } else if (is_cith((unitp_t)node->data)) {
            eval_cith(&result, node, env);
        } else if (is_call((unitp_t)node->data)) {
            eval_call(&result, node, env);
        } else if (is_tila_list((unitp_t)node->data))
            eval_tila_list(&result, node->children, env);
        else if (is_tila_nth((unitp_t)node->data))
            eval_tila_nth(&result, node, env);
    }
    return result;
}
