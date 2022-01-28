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

char *binding_name(char *b) {
  /* semicolon removed from the head */
  char *name = (char *)malloc(strlen(b));
  strncpy(name, b + 1, strlen(b));
  return name;
}
struct Let_data *eval3(GNode *, GHashTable *);

void eval_pret(struct Let_data **result, GNode *root, GHashTable *env) {
  *result = pret(eval3(root->children, env));
}

void eval_lambda(struct Let_data **result, GNode *root, GHashTable *env) {
  struct Lambda *lambda = malloc(sizeof (struct Lambda));
  lambda->expr = g_node_last_child(root);
  lambda->env = env;
  (*result)->type = LAMBDA;
  ((unitp_t)root->data)->lambda_expr = g_node_last_child(root);
  ((unitp_t)root->data)->lambda_env = env;
  /* need type of the unit anymore? we already have the result type
     above... */
  ((unitp_t)root->data)->type = LAMBDA;
  /* add parameters to env */
  for (guint i = 0; i < g_node_n_children(root) - 1; i++) {
    GNode *binding = g_node_nth_child(root, i);
    char *bind_name = ((unitp_t)binding->data)->token.str;
    char *name = binding_name(bind_name);
    /* if it is an optional parameter save it's value */
    if (unit_type((unitp_t)binding->data) == BOUND_BINDING) {
      g_hash_table_insert(((unitp_t)root->data)->lambda_env, name,
			  eval3(binding->children, ((unitp_t)root->data)->env));
      g_hash_table_insert(lambda->env, name,
			  eval3(binding->children, lambda->env));
	}
  }
  (*result)->data.lambda_slot = g_node_last_child(root);
  (*result)->data._lambda_slot = lambda;
}

void eval_assoc(struct Let_data **result, GNode *root, GHashTable *env) {
  for (guint i = 0; i < g_node_n_children(root) - 1; i++) {
    GNode *binding = g_node_nth_child(root, i);
    char *bind_name = ((unitp_t)binding->data)->token.str;
    char *name = binding_name(bind_name);
    g_hash_table_insert(((unitp_t)root->data)->env, name,
			eval3(binding->children, ((unitp_t)root->data)->env));
  }
  (*result) = eval3(g_node_last_child(root), ((unitp_t)root->data)->env);
}

void eval_assign(struct Let_data **result, GNode *root, GHashTable *env) {
  char *name = ((unitp_t)g_node_nth_child(root, 0)->data)->token.str;
  struct Let_data *data = eval3(g_node_nth_child(root, 1), env);
  /* definitions are always saved in the global environment, no
     matter in which environment we are currently */
  g_hash_table_insert(((unitp_t)g_node_get_root(root)->data)->env, name, data);
  (*result)->type = data->type;
  /* which data slot to set */
  switch (data->type) {
  case INTEGER:
    (*result)->data.int_slot = data->data.int_slot; break;
  case FLOAT:
    (*result)->data.float_slot = data->data.float_slot; break;
  case LAMBDA:
    (*result)->data.lambda_slot = data->data.lambda_slot;
    (*result)->data._lambda_slot = data->data._lambda_slot;
    break;
  default: break;
  }
}

void copy_hash_table_entry(gpointer key, gpointer val, gpointer ht) {
  g_hash_table_insert(ht, key, val);
}

void eval_funcall(struct Let_data **result, GNode *root, GHashTable *env) {
  GNode *lambda_node = g_node_last_child(root);
  /* populate lambda environment */
  struct Let_data *x = eval3(lambda_node, ((unitp_t)root->data)->env);
  /* make a copy of the lambda env */
  GHashTable *calltime_env = g_hash_table_new(g_str_hash, g_str_equal);
  /* g_hash_table_foreach(((unitp_t)lambda_node->data)->lambda_env, (GHFunc)copy_hash_table_entry, calltime_env); */
  g_hash_table_foreach(x->data._lambda_slot->env, (GHFunc)copy_hash_table_entry, calltime_env);
  /* iterate over passed arguments */
  gint idx = 0;			/* gint because of g_node_child_index update later */
  for (; idx < (gint)g_node_n_children(root) - 1; idx++) {
    if (unit_type((unitp_t)g_node_nth_child(root, idx)->data) == BOUND_BINDING) {
      g_hash_table_insert(calltime_env,
			  binding_name(((unitp_t)g_node_nth_child(root, idx)->data)->token.str),
			  eval3(g_node_nth_child(root, idx)->children, ((unitp_t)root->data)->env));
      /* update the index to reflect the position of current passed
	 argument in the parameter list of the lambda */
      /* gint in_lambda_idx = g_node_child_index(lambda_node, */
      /* 					      g_hash_table_lookup(((unitp_t)lambda_node->data)->lambda_env, */
      /* 								  binding_name(((unitp_t)g_node_nth_child(root, idx)->data)->token.str))); */
      gint in_lambda_idx = g_node_child_index(lambda_node,
					      g_hash_table_lookup(x->data._lambda_slot->env,
								  binding_name(((unitp_t)g_node_nth_child(root, idx)->data)->token.str)));

      if (in_lambda_idx == idx) idx++;
      else idx = in_lambda_idx;
    } else {			/* just an expression, not a binding (para->arg) */
      char *bname =binding_name(((unitp_t)g_node_nth_child(lambda_node, idx)->data)->token.str);
      g_hash_table_insert(calltime_env, bname,
			  eval3(g_node_nth_child(root, idx), ((unitp_t)root->data)->env));
    }
  }
  /* *result = eval3(g_node_last_child(lambda_node), calltime_env); */
  *result = eval3(x->data._lambda_slot->expr, calltime_env);
}

void eval_toplevel(struct Let_data **result, GNode *root) {
  guint size = g_node_n_children(root);
  for (guint i = 0; i < size - 1; i++)
    eval3(g_node_nth_child(root, i), ((unitp_t)root->data)->env);
  *result = eval3(g_node_nth_child(root, size - 1), ((unitp_t)root->data)->env);
}

struct Let_data *eval3(GNode *root, GHashTable *env) {
  struct Let_data *result = malloc(sizeof (struct Let_data));
  if (((unitp_t)root->data)->is_atomic) {
    switch (unit_type(((unitp_t)root->data))) {
    case INTEGER:
      result->type = INTEGER;
      result->data.int_slot = ((unitp_t)root->data)->ival;
      break;
    case FLOAT:
      result->type = FLOAT;
      result->data.float_slot = ((unitp_t)root->data)->fval;
      break;
    case NAME:
      /* a symbol not contained in a BIND expression (sondern hängt einfach so rum im text) */
      {	  
	char *tokstr = ((unitp_t)root->data)->token.str;
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
	      result->data.lambda_slot = data->data.lambda_slot;
	      result->data._lambda_slot = data->data._lambda_slot;
	      break;
	    default: break;
	    }
	    break;
	} else {
	  GNode *parent = root->parent;
	  while (parent) {		  
	    if ((data = g_hash_table_lookup(((unitp_t)parent->data)->env, tokstr))) {
	      result->type = data->type;
	      switch (result->type) {
	      case INTEGER:
		result->data.int_slot = data->data.int_slot; break;
	      case FLOAT:
		result->data.float_slot = data->data.float_slot; break;
	      case LAMBDA:
		result->data.lambda_slot = data->data.lambda_slot;
		result->data._lambda_slot = data->data._lambda_slot;
		break;
	      default: break;
	      }
	      break;
	    } else {
	      parent = parent->parent;
	    }
	  }
	  if (!parent) {		/* wir sind schon beim parent von global env angekommen */
	    fprintf(stderr, "unbound '%s'\n", tokstr);
	    exit(EXIT_FAILURE);
	  }
	}
      }
      break;
    default: break;		/* undefined */
    }
  } else {			/* block/function */
    if ((((unitp_t)root->data)->uuid == 0)) {
      eval_toplevel(&result, root);
    } else if (is_pret4((unitp_t)root->data)) {
      eval_pret(&result, root, env);
    } else if (is_lambda4((unitp_t)root->data)) {
      eval_lambda(&result, root, env);
    } else if (is_assignment4((unitp_t)root->data)) { /* define */
      eval_assign(&result, root, ((unitp_t)root->data)->env);
    } else if (is_funcall((unitp_t)root->data)) {
      eval_funcall(&result, root, env);
    } else if (is_association4((unitp_t)root->data)) {
      eval_assoc(&result, root, ((unitp_t)root->data)->env);
    }
  }
  return result;
}
