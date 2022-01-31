#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include "type.h"
#include "let_data.h"
#include "token.h"
#include "unit.h"
#include "core.h"
#include "ast.h"
#include "print.h"

char *binding_name(char *b) {
  /* semicolon removed from the head */
  char *name = (char *)malloc(strlen(b));
  strncpy(name, b + 1, strlen(b));
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

struct Let_data *eval3(GNode *, GHashTable *);

void eval_pret(struct Let_data **result, GNode *root, GHashTable *env) {
  *result = pret(eval3(root->children, env));
}

void eval_lambda(struct Let_data **result, GNode *node, GHashTable *env) {
  (*result)->type = LAMBDA;
  struct Lambda *lambda = malloc(sizeof (struct Lambda));
  lambda->env = clone_hash_table(env);
  lambda->node = node;
  lambda->param_list = NULL;
  /* add parameters to env */
  for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
    GNode *binding = g_node_nth_child(node, i);
    char *bind_name = ((unitp_t)binding->data)->token.str;
    char *name = binding_name(bind_name);
    lambda->param_list = g_list_append(lambda->param_list, (unitp_t)binding->data);
    /* if it is an optional parameter save it's value */
    if (unit_type((unitp_t)binding->data) == BOUND_BINDING) {
      g_hash_table_insert(lambda->env, name, eval3(binding->children, lambda->env));
    } else if (unit_type((unitp_t)binding->data) == BINDING) {
      g_hash_table_insert(lambda->env, name, NULL);
    }
  }
  (*result)->data.slot_lambda = lambda;
}

void eval_assoc(struct Let_data **result, GNode *node, GHashTable *env) {
  ((unitp_t)node->data)->env = clone_hash_table(env);
  for (guint i = 0; i < g_node_n_children(node) - 1; i++) {
    GNode *binding = g_node_nth_child(node, i);
    char *bind_name = ((unitp_t)binding->data)->token.str;
    char *name = binding_name(bind_name);
    g_hash_table_insert(((unitp_t)node->data)->env, name,
			eval3(binding->children, ((unitp_t)node->data)->env));
  }
  (*result) = eval3(g_node_last_child(node), ((unitp_t)node->data)->env);
}

void eval_assign(struct Let_data **result, GNode *node, GHashTable *env) {
  char *name = ((unitp_t)g_node_nth_child(node, 0)->data)->token.str;
  struct Let_data *data = eval3(g_node_nth_child(node, 1), env);
  /* definitions are always saved in the global environment, no
     matter in which environment we are currently */
  g_hash_table_insert(((unitp_t)g_node_get_root(node)->data)->env, name, data);
  (*result)->type = data->type;
  /* which data slot to set */
  switch (data->type) {
  case INTEGER:
    (*result)->data.int_slot = data->data.int_slot; break;
  case FLOAT:
    (*result)->data.float_slot = data->data.float_slot; break;
  case LAMBDA:
    (*result)->data.slot_lambda = data->data.slot_lambda;
    break;
  default: break;
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

void eval_funcall(struct Let_data **result, GNode *pass, GHashTable *env) {
  GNode *lambda_node = g_node_last_child(pass);
  /* populate lambda environment */
  /* struct Let_data *x = eval3(lambda_node, ((unitp_t)node->data)->env); */
  struct Let_data *x = eval3(lambda_node, env);
  /* make a copy of the lambda env */
  GHashTable *call_time_env = clone_hash_table(x->data.slot_lambda->env);
  /* iterate over passed arguments */
  gint idx = 0;			/* gint because of g_node_child_index update later */
  while (idx < (gint)g_node_n_children(pass) - 1) {
    if (unit_type((unitp_t)g_node_nth_child(pass, idx)->data) == BOUND_BINDING) {
      g_hash_table_insert(call_time_env,
			  binding_name(((unitp_t)g_node_nth_child(pass, idx)->data)->token.str),
			  eval3(g_node_nth_child(pass, idx)->children, env));
      /* update the index to reflect the position of current passed
	 argument in the parameter list of the lambda */
      gint in_lambda_idx = get_param_index(x->data.slot_lambda->param_list,
					   ((unitp_t)g_node_nth_child(pass, idx)->data)->token.str);;
      if (in_lambda_idx == -1) {
	fprintf(stderr, "unknown parameter\n");
	print_node(g_node_nth_child(pass, idx), NULL);
	fprintf(stderr, "passed to\n");
	print_node(x->data.slot_lambda->node, NULL);
	exit(EXIT_FAILURE);
      } else if (in_lambda_idx > idx) {
	if (unit_type((unitp_t)g_node_nth_child(pass, idx+1)->data) == BOUND_BINDING) {
	  idx++;
	} else 	idx = in_lambda_idx;
      }
      else
	idx++;
    } else {			/* just an expression, not a binding (para->arg) */
      char *bname = binding_name(((unitp_t)g_node_nth_child(x->data.slot_lambda->node, idx)->data)->token.str);
      g_hash_table_insert(call_time_env, bname,
			  eval3(g_node_nth_child(pass, idx), env));
      idx++;
    }
  }
  *result = eval3(g_node_last_child(x->data.slot_lambda->node), call_time_env);
}

void eval_toplevel(struct Let_data **result, GNode *root) {
  guint size = g_node_n_children(root);
  for (guint i = 0; i < size - 1; i++)
    /* every other toplevel part of code inherits the environment of the toplevel */
    eval3(g_node_nth_child(root, i), ((unitp_t)root->data)->env);
  *result = eval3(g_node_nth_child(root, size - 1), ((unitp_t)root->data)->env);
}

struct Let_data *eval3(GNode *node, GHashTable *env) {
  struct Let_data *result = malloc(sizeof (struct Let_data));
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
    case NAME:
      /* a symbol not contained in a BIND expression (sondern hängt einfach so rum im text) */
      {	  
	char *tokstr = ((unitp_t)node->data)->token.str;
	/* symbols are evaluated in the envs of their enclosing units */
	struct Let_data *data;
	if ((data = g_hash_table_lookup(env, tokstr))) {
	    result->type = data->type;
	    switch (result->type) {
	    case INTEGER:
	      result->data.int_slot = data->data.int_slot; break;
	    case FLOAT:
	      result->data.float_slot = data->data.float_slot; break;
	    case LAMBDA:
	      result->data.slot_lambda = data->data.slot_lambda;
	      break;
	    default: break;
	    }
	    break;
	} else {
	  GNode *parent = node->parent;
	  while (parent) {		  
	    if ((data = g_hash_table_lookup(((unitp_t)parent->data)->env, tokstr))) {
	      result->type = data->type;
	      switch (result->type) {
	      case INTEGER:
		result->data.int_slot = data->data.int_slot; break;
	      case FLOAT:
		result->data.float_slot = data->data.float_slot; break;
	      case LAMBDA:
		result->data.slot_lambda = data->data.slot_lambda;
		break;
	      default: break;
	      }
	      break;
	    } else {
	      parent = parent->parent;
	    }
	  }
	  if (!parent) {		/* wir sind schon beim parent von global env angekommen */
	    fprintf(stderr, "lookup failed for\n");
	    print_node(node, NULL);
	    exit(EXIT_FAILURE);
	  }
	}
      }
      break;
    default: break;		/* undefined */
    }
  } else {			/* block/function */
    if ((((unitp_t)node->data)->uuid == 0)) {
      eval_toplevel(&result, node); /* toplevel uses it's own environment */
    } else if (is_pret4((unitp_t)node->data)) {
      eval_pret(&result, node, env);
    } else if (is_lambda4((unitp_t)node->data)) {
      eval_lambda(&result, node, env);
    } else if (is_assignment4((unitp_t)node->data)) { /* define */
      /* eval_assign(&result, node, ((unitp_t)node->data)->env); */
      eval_assign(&result, node, env);
    } else if (is_funcall((unitp_t)node->data)) {
      eval_funcall(&result, node, env);
    } else if (is_association4((unitp_t)node->data)) {
      /* eval_assoc(&result, node, ((unitp_t)node->data)->env); */
      eval_assoc(&result, node, env);
    }
  }
  return result;
}
