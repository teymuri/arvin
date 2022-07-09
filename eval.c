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

void eval_list_op(struct Arv_data **result, GNode *node, GHashTable *env);
void eval_call(struct Arv_data **, GNode *, GHashTable *);
void eval_cpack(struct Arv_data **, GNode *, GHashTable *, guint, guint);

char *binding_node_name(GNode *node) {
    struct Unit *u = ((struct Unit *)node->data);
    char *s = u->token->string;
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

struct Arv_data *
eval3(GNode *, GHashTable *);


void
eval_show_op(struct Arv_data **return_data,
             GNode *node,
             GHashTable *env)
{
    *return_data = eval3(node->children, env);
    switch ((*return_data)->type) {
    case INT: print_int(*return_data); break;
    case FLOAT: print_float(*return_data); break;
    case BOOL: print_bool(*return_data); break;
    case LAMBDA: print_lambda(*return_data); break;
    case LIST: print_list(*return_data); break;
    default:
        printf("Show received unknown data type %d\n", (*return_data)->type);
        break;
    }
    printf("\n");
    free_unit((struct Unit *)node->data); /* also frees token and it's string */
    g_node_destroy(node);
}

gint get_param_index(GList *list, char *str) {
  bool found = false;
  gint idx = 0;
  while (list) {
    if (!strcmp(((struct Unit *)list->data)->token->string, str)) {
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
eval_cond(struct Arv_data **result, GNode *node, GHashTable *env)
{
    guint clause_count = g_node_n_children(node);
    struct Arv_data *d;
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
        lst_len = ((struct Arv_data *)list.item->data)->slots.arv_list->size;
        if (lst_len < min)
            min = lst_len;
        list.item = list.item->next;
    }
    return min;
}

void
pack_evaled_list(struct Arv_data **result, GNode *node)
{
    (*result)->type = LIST;
    struct List *list = (struct List *)malloc(sizeof (struct List));
    list->item = NULL;
    list->size = 0;
    while (node) {
        list->item = g_list_append(list->item, (struct Arv_data *)node->data);
        list->size++;
        node = node->next;      /* node's sibling */
    }
    (*result)->slots.arv_list = list;
}

void
eval_lfold_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Lambda *lambda = eval3(g_node_nth_child(node, 0), env)->slots.tila_lambda;
    struct Arv_data *acc = eval3(g_node_nth_child(node, 1), env);
    /* list is a list of lists */
    struct List *list = eval3(g_node_nth_child(node, 2), env)->slots.arv_list;
    GList *list_item_cp = list->item;
    GHashTable *call_env = clone_hash_table(lambda->env);
    guint minsz = min_sublist_size(*list);
    guint param_idx = 0;
    while (minsz--) {
        while (param_idx < g_list_length(lambda->param_list)) {
            if (unit_type((struct Unit *)g_node_nth_child(lambda->node, param_idx)->data) == PACK_BINDING ||
                unit_type((struct Unit *)g_node_nth_child(lambda->node, param_idx)->data) == BOUND_PACK_BINDING) {
                GNode *args_node = g_node_new(NULL);
                g_node_insert(args_node, -1,
                              g_node_new(param_idx ? ((struct Arv_data *)list->item->data)->slots.arv_list->item->data : acc));
                if (param_idx) {
                    ((struct Arv_data *)list->item->data)->slots.arv_list->item = ((struct Arv_data *)list->item->data)->slots.arv_list->item->next;
                    list->item = list->item->next;
                }
                while (list->item) {
                    g_node_insert(args_node, -1, g_node_new(((struct Arv_data *)list->item->data)->slots.arv_list->item->data));
                    ((struct Arv_data *)list->item->data)->slots.arv_list->item = ((struct Arv_data *)list->item->data)->slots.arv_list->item->next;
                    list->item = list->item->next;
                }
                struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
                pack_evaled_list(&rest_args, args_node->children);
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda->node, param_idx)),
                                    rest_args);
                g_node_destroy(args_node);
                break;
            } else {
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda->node, param_idx)),
                                    param_idx ? ((struct Arv_data *)list->item->data)->slots.arv_list->item->data : acc);
                if (param_idx) {
                    ((struct Arv_data *)list->item->data)->slots.arv_list->item = ((struct Arv_data *)list->item->data)->slots.arv_list->item->next;
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
void
eval_lfold_op2(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Lambda *lambda = eval3(g_node_nth_child(node, 0), env)->slots.tila_lambda;
    struct Arv_data *acc = eval3(g_node_nth_child(node, 1), env);
    /* list is a list of lists */
    /* struct List *list = eval3(g_node_nth_child(node, 2), env)->slots.arv_list; */
    struct List *list = (struct List *)malloc(sizeof (struct List));
    list->size = g_node_n_children(node) - 1 - 1; /* - func - acc */
    int sophie = 0;
    while (sophie < list->size) {
        list->item = g_list_append(list->item, eval3(g_node_nth_child(node, sophie+2), env));
        sophie++;
    }
    GList *list_item_cp = list->item;
    GHashTable *call_env = clone_hash_table(lambda->env);
    guint minsz = min_sublist_size(*list);
    guint param_idx = 0;
    while (minsz--) {
        while (param_idx < g_list_length(lambda->param_list)) {
            if (unit_type((struct Unit *)g_node_nth_child(lambda->node, param_idx)->data) == REST_MAND_PARAM || /*  PACK_BINDING */
                unit_type((struct Unit *)g_node_nth_child(lambda->node, param_idx)->data) == REST_OPT_PARAM) {
                GNode *args_node = g_node_new(NULL);
                g_node_insert(args_node, -1,
                              g_node_new(param_idx ? ((struct Arv_data *)list->item->data)->slots.arv_list->item->data : acc));
                if (param_idx) {
                    ((struct Arv_data *)list->item->data)->slots.arv_list->item = ((struct Arv_data *)list->item->data)->slots.arv_list->item->next;
                    list->item = list->item->next;
                }
                while (list->item) {
                    g_node_insert(args_node, -1, g_node_new(((struct Arv_data *)list->item->data)->slots.arv_list->item->data));
                    ((struct Arv_data *)list->item->data)->slots.arv_list->item = ((struct Arv_data *)list->item->data)->slots.arv_list->item->next;
                    list->item = list->item->next;
                }
                struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
                pack_evaled_list(&rest_args, args_node->children);
                g_hash_table_insert(call_env,
                                    binding_node_name(g_node_nth_child(lambda->node, param_idx)),
                                    rest_args);
                g_node_destroy(args_node);
                break;
            } else {
                g_hash_table_insert(call_env,
                                    /* binding_node_name(g_node_nth_child(lambda->node, param_idx)), */
                                    (char *)g_list_nth(lambda->param_list, param_idx)->data,
                                    param_idx ? ((struct Arv_data *)list->item->data)->slots.arv_list->item->data : acc);
                if (param_idx) {
                    ((struct Arv_data *)list->item->data)->slots.arv_list->item = ((struct Arv_data *)list->item->data)->slots.arv_list->item->next;
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
eval_inc_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n = eval3(node->children, env);
    if (n->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = n->slots.tila_int + 1;
    } else if (n->type == FLOAT) {
        (*result)->type = FLOAT;
        (*result)->slots.tila_float = n->slots.tila_float + 1;
    }
}
void
eval_dec_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n = eval3(node->children, env);
    if (n->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = n->slots.tila_int - 1;
    } else if (n->type == FLOAT) {
        (*result)->type = FLOAT;
        (*result)->slots.tila_float = n->slots.tila_float - 1;
    }
}
void
eval_add_op2(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GNode *child_node = node->children;
    float num = 0.0;
    (*result)->type = INT;
    while (child_node) {
        struct Arv_data *x = eval3(child_node, env);
        if (x->type == FLOAT) {
            (*result)->type = FLOAT;
            num += x->slots.tila_float;
        } else if (x->type == INT) {
            num += x->slots.tila_int;
        }
        child_node = child_node->next;    /* next operand */
    }
    if (((*result)->type == INT)) {
        (*result)->slots.tila_int = (int) num;
    } else if (((*result)->type == FLOAT)) {
        (*result)->slots.tila_float = num;
    }
}    
void
eval_mul_op2(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GNode *child_node = node->children;
    float num = 1.0;
    (*result)->type = INT;
    while (child_node) {
        struct Arv_data *x = eval3(child_node, env);
        if (x->type == FLOAT) {
            (*result)->type = FLOAT;
            num *= x->slots.tila_float;
        } else if (x->type == INT) {
            num *= x->slots.tila_int;
        }
        child_node = child_node->next;    /* next operand */
    }
    if (((*result)->type == INT)) {
        (*result)->slots.tila_int = (int) num;
    } else if (((*result)->type == FLOAT)) {
        (*result)->slots.tila_float = num;
    }
}

void
eval_div_op2(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GNode *child_node = node->children;
    (*result)->type = INT;
    float num;
    struct Arv_data *x = eval3(child_node, env);
    if (x->type == FLOAT) {
        (*result)->type = FLOAT;
        num = x->slots.tila_float;
    } else if (x->type == INT) {
        num = x->slots.tila_int;
    }
    child_node = child_node->next;
    while (child_node) {
        struct Arv_data *x = eval3(child_node, env);
        if (x->type == FLOAT) {
            (*result)->type = FLOAT;
            num /= x->slots.tila_float;
        } else if (x->type == INT) {
            num /= x->slots.tila_int;
        }
        child_node = child_node->next;    /* next operand */
    }
    if (((*result)->type == INT)) {
        (*result)->slots.tila_int = (int) num;
    } else if (((*result)->type == FLOAT)) {
        (*result)->slots.tila_float = num;
    }
}    

void
eval_sub_op2(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GNode *child_node = node->children;
    (*result)->type = INT;
    float num;
    struct Arv_data *x = eval3(child_node, env);
    if (x->type == FLOAT) {
        (*result)->type = FLOAT;
        num = x->slots.tila_float;
    } else if (x->type == INT) {
        num = x->slots.tila_int;
    }
    child_node = child_node->next;
    while (child_node) {
        struct Arv_data *x = eval3(child_node, env);
        if (x->type == FLOAT) {
            (*result)->type = FLOAT;
            num -= x->slots.tila_float;
        } else if (x->type == INT) {
            num -= x->slots.tila_int;
        }
        child_node = child_node->next;    /* next operand */
    }
    if (((*result)->type == INT)) {
        (*result)->slots.tila_int = (int) num;
    } else if (((*result)->type == FLOAT)) {
        (*result)->slots.tila_float = num;
    }
}    

void
eval_add_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n1 = eval3(g_node_nth_child(node, 0), env);
    struct Arv_data *n2 = eval3(g_node_nth_child(node, 1), env);
    if (n1->type == INT && n2->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = n1->slots.tila_int + n2->slots.tila_int;
    } else {
        (*result)->type = FLOAT;
        if (n1->type == INT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_int + n2->slots.tila_float;
        else if (n1->type == FLOAT && n2->type == INT)
            (*result)->slots.tila_float = n1->slots.tila_float + n2->slots.tila_int;
        else if (n1->type == FLOAT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_float + n2->slots.tila_float;
        else {
            fprintf(stderr, "passed non-nums to add\n");
            exit(EXIT_FAILURE);            
        }
    }
}

void
eval_mul_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n1 = eval3(g_node_nth_child(node, 0), env);
    struct Arv_data *n2 = eval3(g_node_nth_child(node, 1), env);
    if (n1->type == INT && n2->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = n1->slots.tila_int * n2->slots.tila_int;
    } else {
        (*result)->type = FLOAT;
        if (n1->type == INT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_int * n2->slots.tila_float;
        else if (n1->type == FLOAT && n2->type == INT)
            (*result)->slots.tila_float = n1->slots.tila_float * n2->slots.tila_int;
        else if (n1->type == FLOAT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_float * n2->slots.tila_float;
        else {
            fprintf(stderr, "passed non-nums to mul\n");
            exit(EXIT_FAILURE);            
        }
    }
}

void
eval_sub_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n1 = eval3(g_node_nth_child(node, 0), env);
    struct Arv_data *n2 = eval3(g_node_nth_child(node, 1), env);
    if (n1->type == INT && n2->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = n1->slots.tila_int - n2->slots.tila_int;
    } else {
        (*result)->type = FLOAT;
        if (n1->type == INT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_int - n2->slots.tila_float;
        else if (n1->type == FLOAT && n2->type == INT)
            (*result)->slots.tila_float = n1->slots.tila_float - n2->slots.tila_int;
        else if (n1->type == FLOAT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_float - n2->slots.tila_float;
        else {
            fprintf(stderr, "passed non-nums to sub\n");
            exit(EXIT_FAILURE);            
        }
    }
}

void
eval_div_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n1 = eval3(g_node_nth_child(node, 0), env);
    struct Arv_data *n2 = eval3(g_node_nth_child(node, 1), env);
    if (n1->type == INT && n2->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = n1->slots.tila_int / n2->slots.tila_int;
    } else {
        (*result)->type = FLOAT;
        if (n1->type == INT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_int / n2->slots.tila_float;
        else if (n1->type == FLOAT && n2->type == INT)
            (*result)->slots.tila_float = n1->slots.tila_float / n2->slots.tila_int;
        else if (n1->type == FLOAT && n2->type == FLOAT)
            (*result)->slots.tila_float = n1->slots.tila_float / n2->slots.tila_float;
        else {
            fprintf(stderr, "passed non-nums to div\n");
            exit(EXIT_FAILURE);            
        }
    }
}

void
eval_exp_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *n1 = eval3(g_node_nth_child(node, 0), env);
    struct Arv_data *n2 = eval3(g_node_nth_child(node, 1), env);
    if (n1->type == INT && n2->type == INT) {
        (*result)->type = INT;
        (*result)->slots.tila_int = (int)powf(n1->slots.tila_int, n2->slots.tila_int);
    } else {
        (*result)->type = FLOAT;
        if (n1->type == INT && n2->type == FLOAT)
            (*result)->slots.tila_float = powf(n1->slots.tila_int, n2->slots.tila_float);
        else if (n1->type == FLOAT && n2->type == INT)
            (*result)->slots.tila_float = powf(n1->slots.tila_float, n2->slots.tila_int);
        else if (n1->type == FLOAT && n2->type == FLOAT)
            (*result)->slots.tila_float = powf(n1->slots.tila_float, n2->slots.tila_float);
        else {
            fprintf(stderr, "passed non-nums to exp\n");
            exit(EXIT_FAILURE);
        }
    }
}

/* /\* void *\/ */
/* /\* eval_add_op(struct Arv_data **result, GNode *node, GHashTable *env) *\/ */
/* /\* { *\/ */
/* /\*     struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.arv_list; *\/ */
/* /\*     if (lst->size) { *\/ */
/* /\*         bool allints = true; *\/ */
/* /\*         float acc; *\/ */
/* /\*         struct Arv_data *d = lst->item->data; *\/ */
/* /\*         if (d->type == INT) { *\/ */
/* /\*             acc = (float)d->slots.tila_int; *\/ */
/* /\*         } else if (d->type == FLOAT) { *\/ */
/* /\*             allints = false; *\/ */
/* /\*             acc = d->slots.tila_float; *\/ */
/* /\*         } *\/ */
/* /\*         lst->item = lst->item->next; *\/ */
/* /\*         while (lst->item) { *\/ */
/* /\*             d = lst->item->data; *\/ */
/* /\*             if (d->type == FLOAT) { *\/ */
/* /\*                 acc += d->slots.tila_float; *\/ */
/* /\*                 allints = false; *\/ */
/* /\*             } else if (d->type == INT) *\/ */
/* /\*                 acc += d->slots.tila_int; *\/ */
/* /\*             else { *\/ */
/* /\*                 fprintf(stderr, "non-num to add\n"); *\/ */
/* /\*                 exit(EXIT_FAILURE);             *\/ */
/* /\*             } *\/ */
/* /\*             lst->item = lst->item->next; *\/ */
/* /\*         } *\/ */
/* /\*         if (allints) { *\/ */
/* /\*             (*result)->type = INT; *\/ */
/* /\*             (*result)->slots.tila_int = (int)acc; *\/ */
/* /\*         } *\/ */
/* /\*         else { *\/ */
/* /\*             (*result)->type = FLOAT; *\/ */
/* /\*             (*result)->slots.tila_float = acc; *\/ */
/* /\*         }         *\/ */
/* /\*     } else { *\/ */
/* /\*         fprintf(stderr, "empty list passed to Add op\n"); *\/ */
/* /\*         exit(EXIT_FAILURE);             *\/ */
/* /\*     } *\/ */
/* /\* } *\/ */


/* void */
/* eval_mul_op(struct Arv_data **result, GNode *node, GHashTable *env) */
/* { */
/*     struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.arv_list; */
/*     if (lst->size) { */
/*         bool allints = true; */
/*         float acc; */
/*         struct Arv_data *d = lst->item->data; */
/*         if (d->type == INT) { */
/*             acc = (float)d->slots.tila_int; */
/*         } else if (d->type == FLOAT) { */
/*             allints = false; */
/*             acc = d->slots.tila_float; */
/*         } */
/*         lst->item = lst->item->next; */
/*         while (lst->item) { */
/*             d = lst->item->data; */
/*             if (d->type == FLOAT) { */
/*                 acc *= d->slots.tila_float; */
/*                 allints = false; */
/*             } else if (d->type == INT) */
/*                 acc *= d->slots.tila_int; */
/*             else { */
/*                 fprintf(stderr, "non-num to Mul\n"); */
/*                 exit(EXIT_FAILURE);             */
/*             } */
/*             lst->item = lst->item->next; */
/*         } */
/*         if (allints) { */
/*             (*result)->type = INT; */
/*             (*result)->slots.tila_int = (int)acc; */
/*         } */
/*         else { */
/*             (*result)->type = FLOAT; */
/*             (*result)->slots.tila_float = acc; */
/*         }         */
/*     } else { */
/*         fprintf(stderr, "empty list passed to Mul op\n"); */
/*         exit(EXIT_FAILURE);             */
/*     } */
/* } */


/* void */
/* eval_exp_op(struct Arv_data **result, GNode *node, GHashTable *env) */
/* { */
/*     struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.arv_list; */
/*     if (lst->size) { */
/*         bool allints = true; */
/*         float acc; */
/*         /\* get the first number in the list *\/ */
/*         struct Arv_data *d = lst->item->data; */
/*         if (d->type == INT) { */
/*             acc = (float)d->slots.tila_int; */
/*         } else if (d->type == FLOAT) { */
/*             allints = false; */
/*             acc = d->slots.tila_float; */
/*         } */
/*         lst->item = lst->item->next; */
/*         while (lst->item) { */
/*             d = lst->item->data; */
/*             if (d->type == FLOAT) { */
/*                 acc = powf(acc, d->slots.tila_float); */
/*                 /\* acc -= d->slots.tila_float; *\/ */
/*                 allints = false; */
/*             } else if (d->type == INT) */
/*                 acc = powf(acc, d->slots.tila_int); */
/*                 /\* acc -= d->slots.tila_int; *\/ */
/*             else { */
/*                 fprintf(stderr, "non-num to Exp op\n"); */
/*                 exit(EXIT_FAILURE);             */
/*             } */
/*             lst->item = lst->item->next; */
/*         } */
/*         if (allints) { */
/*             (*result)->type = INT; */
/*             (*result)->slots.tila_int = (int)acc; */
/*         } */
/*         else { */
/*             (*result)->type = FLOAT; */
/*             (*result)->slots.tila_float = acc; */
/*         }         */
/*     } else { */
/*         fprintf(stderr, "empty list passed to Exp op\n"); */
/*         exit(EXIT_FAILURE);             */
/*     } */
/* } */


/* void */
/* eval_sub_op(struct Arv_data **result, GNode *node, GHashTable *env) */
/* { */
/*     struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.arv_list; */
/*     if (lst->size) { */
/*         bool allints = true; */
/*         float acc; */
/*         struct Arv_data *d = lst->item->data; */
/*         if (d->type == INT) { */
/*             acc = (float)d->slots.tila_int; */
/*         } else if (d->type == FLOAT) { */
/*             allints = false; */
/*             acc = d->slots.tila_float; */
/*         } */
/*         lst->item = lst->item->next; */
/*         while (lst->item) { */
/*             d = lst->item->data; */
/*             if (d->type == FLOAT) { */
/*                 acc -= d->slots.tila_float; */
/*                 allints = false; */
/*             } else if (d->type == INT) */
/*                 acc -= d->slots.tila_int; */
/*             else { */
/*                 fprintf(stderr, "non-num to add\n"); */
/*                 exit(EXIT_FAILURE);             */
/*             } */
/*             lst->item = lst->item->next; */
/*         } */
/*         if (allints) { */
/*             (*result)->type = INT; */
/*             (*result)->slots.tila_int = (int)acc; */
/*         } */
/*         else { */
/*             (*result)->type = FLOAT; */
/*             (*result)->slots.tila_float = acc; */
/*         }         */
/*     } else { */
/*         fprintf(stderr, "empty list passed to sub op\n"); */
/*         exit(EXIT_FAILURE);             */
/*     } */
/* } */
/* void */
/* eval_div_op(struct Arv_data **result, GNode *node, GHashTable *env) */
/* { */
/*     struct List *lst = eval3(g_node_nth_child(node, 0), env)->slots.arv_list; */
/*     if (lst->size) { */
/*         bool allints = true; */
/*         float acc; */
/*         struct Arv_data *d = lst->item->data; */
/*         if (d->type == INT) { */
/*             acc = (float)d->slots.tila_int; */
/*         } else if (d->type == FLOAT) { */
/*             allints = false; */
/*             acc = d->slots.tila_float; */
/*         } */
/*         lst->item = lst->item->next; */
/*         while (lst->item) { */
/*             d = lst->item->data; */
/*             if (d->type == FLOAT) { */
/*                 acc /= d->slots.tila_float; */
/*                 allints = false; */
/*             } else if (d->type == INT) */
/*                 acc /= d->slots.tila_int; */
/*             else { */
/*                 fprintf(stderr, "non-num to add\n"); */
/*                 exit(EXIT_FAILURE);             */
/*             } */
/*             lst->item = lst->item->next; */
/*         } */
/*         if (allints) { */
/*             (*result)->type = INT; */
/*             (*result)->slots.tila_int = (int)acc; */
/*         } */
/*         else { */
/*             (*result)->type = FLOAT; */
/*             (*result)->slots.tila_float = acc; */
/*         }         */
/*     } else { */
/*         fprintf(stderr, "empty list passed to div op\n"); */
/*         exit(EXIT_FAILURE);             */
/*     } */
/* } */





/* ******** math end ******** */
/* ******* begin list ******* */
void
eval_list_op(struct Arv_data **result, GNode *node, GHashTable *env)
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
    (*result)->slots.arv_list = list;
}
void
eval_list_op2(struct Arv_data **result, GNode *node, GHashTable *env)
/* node is the list node (not it's first item) */
{
    (*result)->type = LIST;
    struct List *list = (struct List *)malloc(sizeof (struct List));
    list->item = NULL;
    list->size = 0;
    GNode *child = node->children;
    while (child) {
        /* each item of a list is evaluated at list creation-time */
        list->item = g_list_append(list->item, eval3(child, env));
        list->size++;
        child = child->next;      /* next list item */
    }
    (*result)->slots.arv_list = list;
}

void
eval_nth_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    guint idx = eval3(g_node_nth_child(node, 0), env)->slots.tila_int;
    struct List *list = eval3(g_node_nth_child(node, 1), env)->slots.arv_list;
    struct Arv_data *data = g_list_nth(list->item, idx)->data; /* item = first link of list */
    set_data(*result, data);
}

void eval_size_op(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *d = eval3(node->children, env);
    /* work only if data is list */
    if (d->type == LIST) {
        (*result)->type = INT;
        (*result)->slots.tila_int = d->slots.arv_list->size;
    } else {
        fprintf(stderr, "size arg must eval to list\n");
        print_node(node->children, NULL);
        exit(EXIT_FAILURE);   
    }
}

/* ***** end list ******* */


void eval_lambda(struct Arv_data **result, GNode *node, GHashTable *env)
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
        if (unit_type((struct Unit *)binding->data) == BOUND_BINDING ||
            /* default arg to a rest param is a single list */
            unit_type((struct Unit *)binding->data) == BOUND_PACK_BINDING) {
            g_hash_table_insert(lambda->env, name, eval3(binding->children, lambda->env));
        } else if (unit_type((struct Unit *)binding->data) == BINDING ||
                   unit_type((struct Unit *)binding->data) == PACK_BINDING) {
            g_hash_table_insert(lambda->env, name, NULL);
        }
    }
    (*result)->slots.tila_lambda = lambda;
}

void eval_lambda2(struct Arv_data **return_data, GNode *node, GHashTable *env)
{
    (*return_data)->type = LAMBDA;
    struct Lambda *lambda = malloc(sizeof (struct Lambda));
    /* make a snapshot of the current environment at the time of
     * lambda creation */
    lambda->env = clone_hash_table(env);
    lambda->node = node;
    lambda->artyp = ((struct Unit *)node->data)->type;
    lambda->param_list = NULL;
    /* add parameters to env */
    for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
        /* lambda->param_list = g_list_append(lambda->param_list, (struct Unit *)g_node_nth_child(node, i)->data); */
        if (((struct Unit *)g_node_nth_child(node, i)->data)->type == OPT_PARAM) {
            g_hash_table_insert(lambda->env, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 1,
                                eval3(g_node_nth_child(node, i)->children, lambda->env));
            lambda->param_list = g_list_append(lambda->param_list, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 1);
        } else if (((struct Unit *)g_node_nth_child(node, i)->data)->type == REST_OPT_PARAM) {
            g_hash_table_insert(lambda->env, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 2,
                                eval3(g_node_nth_child(node, i)->children, lambda->env));
            lambda->param_list = g_list_append(lambda->param_list, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 2);
        } else if (((struct Unit *)g_node_nth_child(node, i)->data)->type == MAND_PARAM) {
            g_hash_table_insert(lambda->env, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string, NULL);
            lambda->param_list = g_list_append(lambda->param_list, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string);
        } else if (((struct Unit *)g_node_nth_child(node, i)->data)->type == REST_MAND_PARAM) {
            g_hash_table_insert(lambda->env, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 1, NULL);
            lambda->param_list = g_list_append(lambda->param_list, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 1);
        } else {
            fprintf(stderr, "malformed lambda param\n");
            exit(EXIT_FAILURE);
        }
        
        /* if (((struct Unit *)g_node_nth_child(node, i)->data)->type == OPT_PARAM || */
        /*     ((struct Unit *)g_node_nth_child(node, i)->data)->type == REST_OPT_PARAM) { */
        /*     g_hash_table_insert(lambda->env, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string + 1, */
        /*                         eval3(g_node_nth_child(node, i)->children, lambda->env)); */
        /* } else if (((struct Unit *)g_node_nth_child(node, i)->data)->type == MAND_PARAM || */
        /*            ((struct Unit *)g_node_nth_child(node, i)->data)->type == REST_MAND_PARAM) { */
        /*     g_hash_table_insert(lambda->env, ((struct Unit *)g_node_nth_child(node, i)->data)->token->string, NULL); */
        /* } else { */
        /*     fprintf(stderr, "malformed lambda param\n"); */
        /*     exit(EXIT_FAILURE); */
        /* } */
    }
    (*return_data)->slots.tila_lambda = lambda;
}



/* pack contains Arv_data pointers */
/* pack start upto end children, when end == 0 packt alle kinder bis zum ende */
void eval_cpack(struct Arv_data **result, GNode *node,
                GHashTable *env, guint start, guint end) {
    guint n = end ? end : g_node_n_children(node);
    GList *pack = NULL;
    for (guint i = start; i < n; i++)
        pack = g_list_append(pack, eval3(g_node_nth_child(node, i), env));
    (*result)->type = PACK;
    (*result)->slots.pack = pack;
}


void eval_named_rest_args(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GList *pack = NULL;
    guint count = g_node_n_children(node);
    for (guint n = 0; n < count; n++)
        pack = g_list_append(pack, eval3(g_node_nth_child(node, n), env));
    (*result)->type = PACK;
    (*result)->slots.pack = pack;
}
void eval_unnamed_rest_args(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GList *pack = NULL;
    while (node) {
        pack = g_list_append(pack, eval3(node, env));
        node = node->next;      /* node's sibling */
    }
    (*result)->type = PACK;
    (*result)->slots.pack = pack;
}

void eval_let(struct Arv_data **result, GNode *node, GHashTable *env) {
    ((struct Unit *)node->data)->env = clone_hash_table(env);
    for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
        GNode *binding = g_node_nth_child(node, i);
        char *name = binding_node_name(binding);
        g_hash_table_insert(((struct Unit *)node->data)->env, name,
                            eval3(binding->children, ((struct Unit *)node->data)->env));
    }
    (*result) = eval3(g_node_last_child(node), ((struct Unit *)node->data)->env);
}

void eval_let2(struct Arv_data **result, GNode *node, GHashTable *env) {
    if (g_node_n_children(node) % 2 == 0) {
        fprintf(stderr, "malformed let; no expressions found\n");
        exit(EXIT_FAILURE);
    }
    ((struct Unit *)node->data)->env = clone_hash_table(env);
    guint bind_count = g_node_n_children(node) / 2;
    for (guint i = 0; i < bind_count; i++)
        g_hash_table_insert(((struct Unit *)node->data)->env,
                            ((struct Unit *)g_node_nth_child(node, i * 2)->data)->token->string,
                            eval3(g_node_nth_child(node, i * 2 + 1), ((struct Unit *)node->data)->env));
    (*result) = eval3(g_node_last_child(node), ((struct Unit *)node->data)->env);
}


void
eval_define(struct Arv_data **return_data,
            GNode *node,
            GHashTable *env)
{
    /* Define <name> <expr> */
    /* definitions are always saved in the global environment, no
       matter in which environment we are currently */
    char *name = ((struct Unit *)g_node_nth_child(node, 0)->data)->token->string;
    GHashTable *global_env = ((struct Unit *)g_node_get_root(node)->data)->env;
    *return_data = eval3(g_node_nth_child(node, 1), env);
    if (g_hash_table_contains(global_env, name)) {
        fprintf(stderr, "%s is bound\n", name);
        exit(EXIT_FAILURE);
    } else g_hash_table_insert(global_env, name, *return_data);
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

gint find_param_idx2(GList *param_lst_lnk, char *str) {
    bool found = false;
    gint idx = 0;
    while (param_lst_lnk) {
        /* if (!strcmp(((struct Unit *)param_lst_lnk->data)->type == OPT_PARAM ? ((struct Unit *)param_lst_lnk->data)->token->string+1 : ((struct Unit *)param_lst_lnk->data)->token->string, str)) */
        if (!strcmp((char *)param_lst_lnk->data, str)) {
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

void eval_call(struct Arv_data **result, GNode *node, GHashTable *env)
{
    GNode *lambda_node = g_node_first_child(node);
    struct Arv_data *lambda_data = eval3(lambda_node, env);
    /* make a copy of the lambda env just for this call */
    GHashTable *call_env = clone_hash_table(lambda_data->slots.tila_lambda->env);
    guint arg_idx = 0;
    gint param_idx = 0;
    GNode *first_arg = g_node_nth_child(node, 1);
    /* iterate over passed arguments */
    while (arg_idx < g_node_n_children(node) - 1) {
        if (unit_type((struct Unit *)nth_sibling(first_arg, arg_idx)->data) == BOUND_BINDING) {
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
        } else if (is_of_type((struct Unit *)nth_sibling(first_arg, arg_idx)->data, BOUND_PACK_BINDING)) {
            struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
            /* named rest args */
            /* eval the passed arguments in the  */
            eval_list_op(&rest_args, nth_sibling(first_arg, arg_idx)->children, env);
            g_hash_table_insert(call_env,
                                binding_node_name(nth_sibling(first_arg, arg_idx)),
                                rest_args);
            break;       /* out of parameter processing, &rest: must be the last! */
        } else {			/* just an expression or multiple expressions, not a binding (para->arg) */
            if (unit_type((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data) == PACK_BINDING ||
                unit_type((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data) == BOUND_PACK_BINDING) {
                /* reached rest args without param name */
                struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
                eval_list_op(&rest_args, nth_sibling(first_arg, arg_idx), env);
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

void
eval_call2(struct Arv_data **result, GNode *node, GHashTable *env)
{
    struct Arv_data *lambda_data = eval3(g_node_first_child(node), env);
    /* make a copy of the lambda env just for this call */
    GHashTable *call_env = clone_hash_table(lambda_data->slots.tila_lambda->env);
    guint arg_idx = 1;          /* 0th is the function */
    gint param_idx = 0;
    GNode *first_arg = g_node_nth_child(node, 1);
    /* iterate over passed arguments */
    while (arg_idx < g_node_n_children(node)) {
        if (((struct Unit *)g_node_nth_child(node, arg_idx)->data)->type == OPT_PARAM) {
            g_hash_table_insert(call_env,
                                /* binding_node_name(nth_sibling(first_arg, arg_idx)), */
                                ((struct Unit *)g_node_nth_child(node, arg_idx)->data)->token->string + 1, /* @param without @ */
                                eval3(g_node_nth_child(node, arg_idx)->children, env));
            
            param_idx = find_param_idx2(lambda_data->slots.tila_lambda->param_list,
                                        /* binding_node_name(nth_sibling(first_arg, arg_idx)) */
                                        ((struct Unit *)g_node_nth_child(node, arg_idx)->data)->token->string + 1);

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
        }
        /* *****************deprecated********************** */
        else if (is_of_type((struct Unit *)nth_sibling(first_arg, arg_idx)->data, BOUND_PACK_BINDING)) { /*  */
            struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
            /* named rest args */
            /* eval the passed arguments in the  */
            eval_list_op(&rest_args, nth_sibling(first_arg, arg_idx)->children, env);
            g_hash_table_insert(call_env,
                                binding_node_name(nth_sibling(first_arg, arg_idx)),
                                rest_args);
            break;       /* out of parameter processing, &rest: must be the last! */
        }
        else if (((struct Unit *)g_node_nth_child(node, arg_idx)->data)->type == CALL_OPT_REST_PARAM) {
            /* rest args with param name, these are the last args passed to the function */
            struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
            GNode *rest_node = g_node_new(NULL);
            guint rest_arg_idx = 0;
            while (rest_arg_idx < g_node_n_children(g_node_nth_child(node, arg_idx))) {
                g_node_insert(rest_node,-1, g_node_copy(g_node_nth_child(g_node_nth_child(node, arg_idx), rest_arg_idx)));
                rest_arg_idx++;
            }
            eval_list_op2(&rest_args, rest_node, env);
            param_idx = find_param_idx2(lambda_data->slots.tila_lambda->param_list,
                                        ((struct Unit *)g_node_nth_child(node, arg_idx)->data)->token->string + 2);
            if (param_idx == -1) {
                fprintf(stderr, "unknown parameter\n");
                print_node(nth_sibling(first_arg, arg_idx), NULL);
                fprintf(stderr, "passed to\n");
                print_node(lambda_data->slots.tila_lambda->node, NULL);
                exit(EXIT_FAILURE);
            } else {
                g_hash_table_insert(call_env,
                                    g_list_nth_data(lambda_data->slots.tila_lambda->param_list, param_idx),
                                    rest_args);
            }
            break;                  /* and that was the last param; done, get out!!! */
        }
        else {			/* just an expression or multiple expressions, not a binding (@para arg or @&rest_param arg1 ... argN) */
            if (unit_type((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data) == REST_MAND_PARAM ||
                unit_type((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data) == REST_OPT_PARAM) {
                /* reached rest args without param name */
                struct Arv_data *rest_args = malloc(sizeof (struct Arv_data));
                GNode *rest_node = g_node_new(NULL);
                while (arg_idx < g_node_n_children(node)) {
                    g_node_insert(rest_node,-1, g_node_copy(g_node_nth_child(node, arg_idx)));
                    arg_idx++;
                }
                eval_list_op2(&rest_args, rest_node, env);
                g_hash_table_insert(call_env,
                                    g_list_nth_data(lambda_data->slots.tila_lambda->param_list, param_idx),
                                    rest_args);
                break;                  /* and that was the last param; done, get out!!! */
            } else {
                /* ((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data)->type == MAND_PARAM ? */
                /*     ((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data)->token->string : */
                /*     ((struct Unit *)g_node_nth_child(lambda_data->slots.tila_lambda->node, param_idx)->data)->token->string + 1, */

                g_hash_table_insert(call_env,
                                    g_list_nth_data(lambda_data->slots.tila_lambda->param_list, param_idx),
                                    eval3(g_node_nth_child(node, arg_idx), env));
                arg_idx++;
                param_idx++;
            }
        }
    }
    /* eval lambda's expression in the created call-time env */
    *result = eval3(g_node_last_child(lambda_data->slots.tila_lambda->node), call_env);
}

void eval_toplvl(struct Arv_data **result, GNode *root)
{
    guint size = g_node_n_children(root);
    for (guint i = 0; i < size - 1; i++)
        /* every other toplevel part of code inherits the environment of the toplevel */
        eval3(g_node_nth_child(root, i), ((struct Unit *)root->data)->env);
    *result = eval3(g_node_nth_child(root, size - 1), ((struct Unit *)root->data)->env);
}

struct Arv_data *
eval3(GNode *node, GHashTable *env)
{
    struct Arv_data *return_data = malloc(sizeof (struct Arv_data));
    if (((struct Unit *)node->data)->is_atomic) {
        switch (unit_type(((struct Unit *)node->data))) {
        case INT:
            return_data->type = INT;
            return_data->slots.tila_int = ((struct Unit *)node->data)->ival;
            break;
        case FLOAT:
            return_data->type = FLOAT;
            return_data->slots.tila_float = ((struct Unit *)node->data)->fval;
            break;
        case BOOL:              /* a literal boolean (i.e. true/false) */
            return_data->type = BOOL;
            return_data->slots.tila_bool = is_true((struct Unit *)node->data) ? true : false;
            break;
        case NAME:
        {
            char *wanted_kw = ((struct Unit *)node->data)->token->string;
            /* symbols are evaluated in the envs of their enclosing units */
            GNode *nodecp = node; /* copy node for print_node belowv */
            do {            
                if ((return_data = g_hash_table_lookup(env, wanted_kw))) break;
                /* the && env = ... here is just for the purpose of
                 * the assignment itself (supposing the node
                 * assignment and check part happened to be
                 * successful) and not really a condition check */
            } while ((node = node->parent) && (env = ((struct Unit *)node->data)->env));
            if (!node) {
                fprintf(stderr, "lookup failed for\n");
                print_node(nodecp, NULL);
                exit(EXIT_FAILURE);
            }
        } break;                  /* break the NAME branch */
        default: break;		/* undefined unit type */
        }
    } else {			/* builtin stuff */
        if ((((struct Unit *)node->data)->uuid == 0)) {
            eval_toplvl(&return_data, node); /* toplevel uses it's own environment */
        } else if (is_show_op((struct Unit *)node->data))
            eval_show_op(&return_data, node, env);
        /* else if (is_lambda4((struct Unit *)node->data)) { */
        /*     eval_lambda(&return_data, node, env); */
        /* } */
        else if (is_define((struct Unit *)node->data)) { /* define */
            eval_define(&return_data, node, env);
        }
        /* else if (is_let((struct Unit *)node->data)) { */
        /*     eval_let(&return_data, node, env); */
        /* } */
        else if (is_cpack((struct Unit *)node->data)) {
            eval_cpack(&return_data, node, env, 0, 0);
        }
        /* else if (is_call((struct Unit *)node->data)) { */
        /*     for (int i = 0; i < ((struct Unit *)node->data)->call_rpt_cnt; i++) */
        /*         eval_call(&return_data, node, env); */
        /* } */
        else if (is_call2((struct Unit *)node->data))
            eval_call2(&return_data, node, env);
        else if (is_lambda((struct Unit *)node->data))
            eval_lambda2(&return_data, node, env);
        else if (is_nth_op((struct Unit *)node->data))
            eval_nth_op(&return_data, node, env);
        else if (is_size_op((struct Unit *)node->data))
            eval_size_op(&return_data, node, env);
        /* else if (is_list_op((struct Unit *)node->data)) */
        /*     /\* List is used in the core ONLY as default argument the */
        /*      * &ITEMS:= param of the list function to get an empty */
        /*      * list. although invoking it would simply return_data in an */
        /*      * empty list (as it's max capacity is 0 it cant take any */
        /*      * args) try NOT to use it elsewhere as it's internal and */
        /*      * can change without notice! *\/ */
        /*     eval_list_op(&return_data, node->children, env); */
        else if (is_list_op2((struct Unit *)node->data))
            eval_list_op2(&return_data, node, env);
        else if (is_cond((struct Unit *)node->data))
            eval_cond(&return_data, node, env);
        else if (is_add_op2((struct Unit *)node->data))
            eval_add_op2(&return_data, node, env);
        else if (is_mul_op2((struct Unit *)node->data))
            eval_mul_op2(&return_data, node, env);
        else if (is_sub_op2((struct Unit *)node->data))
            eval_sub_op2(&return_data, node, env);
        else if (is_div_op2((struct Unit *)node->data))
            eval_div_op2(&return_data, node, env);
        else if (is_add_op((struct Unit *)node->data))
            eval_add_op(&return_data, node, env);
        else if (is_sub_op((struct Unit *)node->data))
            eval_sub_op(&return_data, node, env);
        else if (is_mul_op((struct Unit *)node->data))
            eval_mul_op(&return_data, node, env);
        else if (is_div_op((struct Unit *)node->data))
            eval_div_op(&return_data, node, env);
        else if (is_exp_op((struct Unit *)node->data))
            eval_exp_op(&return_data, node, env);
        else if (is_inc_op((struct Unit *)node->data))
            eval_inc_op(&return_data, node, env);
        else if (is_dec_op((struct Unit *)node->data))
            eval_dec_op(&return_data, node, env);
        /* else if (is_lfold_op((struct Unit *)node->data)) */
        /*     eval_lfold_op(&return_data, node, env); */
        else if (is_lfold_op2((struct Unit *)node->data))
            eval_lfold_op2(&return_data, node, env);
        else if (is_let2((struct Unit *)node->data))
            eval_let2(&return_data, node, env);
    }
    return return_data;
}
