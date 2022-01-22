#include <stdlib.h>		/* EXIT_FAILURE */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "type.h"
#include "env.h"
#include "token.h"
#include "unit.h"

#include "core.h"

#include "print.h"

#define LEAST_COL_START_IDX -2

bool is_enclosed_in3(GSList *ulink1, GSList *ulink2) {
  if (ulink2 != NULL) {
    struct Unit *u1 = (unitp_t)ulink1->data, *u2 = (unitp_t)ulink2->data;
    return u1->token.col_start_idx > u2->token.col_start_idx &&
      u1->token.line >= u2->token.line;
  } else {
    return false;
  }
}
bool is_enclosed_in4(struct Unit *a1, struct Unit *a2) {
  return a1->token.col_start_idx > a2->token.col_start_idx &&
    a1->token.line >= a2->token.line;
}

int bottom_line_number3(GSList *list) {
  int line = -1;
  while (list) {
    if (((unitp_t)list->data)->token.line > line)
      line = ((unitp_t)list->data)->token.line;
    list = list->next;
  }
  return line;
}

GSList *enclosures3(GSList *ulink, GSList *list) {
  GSList *sll = NULL;
  for (;
       list && ((unitp_t)list->data)->uuid < ((unitp_t)ulink->data)->uuid;
       list = list->next) {
    if (!((unitp_t)list->data)->is_atomic && is_enclosed_in3(ulink, list))
      sll = g_slist_append(sll, list->data);
  }
  return sll;
}

/* remove from encs */
GSList *bottom_enclosures3(GSList *encs) {
  int bot_linum = bottom_line_number3(encs);
  GSList *out = NULL;
  while (encs) {
    if (((unitp_t)encs->data)->token.line == bot_linum)
      out = g_slist_append(out, encs->data);
    encs = encs->next;
  }
  return out;
}
GSList *rightmost_enclosure(GSList *botencs) {
  int col_start_idx = LEAST_COL_START_IDX;
  GSList *rmost = NULL;
  while (botencs) {
    if (((unitp_t)botencs->data)->token.col_start_idx > col_start_idx) {
      col_start_idx = ((unitp_t)botencs->data)->token.col_start_idx;
      rmost = botencs;
    }
    botencs = botencs->next;
  }
  return rmost;
}

GSList *find_enclosure_link(GSList *unit_link, GSList *units_onset) {
  GSList *all = enclosures3(unit_link, units_onset);
  GSList *undermosts = bottom_enclosures3(all);
  return rightmost_enclosure(undermosts);
}


/* to be included also in eval */
bool is_lambda_unit(struct Unit *u)
{
  return !strcmp(u->token.str, LAMBDA_KEYWORD);
}

bool is_lambda_head(struct Unit c) { return !strcmp(c.token.str, LAMBDA_KEYWORD); }
bool is_lambda3(GSList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, LAMBDA_KEYWORD);
}


bool is_lambda4(struct Unit *u) {
  return !strcmp(u->token.str, LAMBDA_KEYWORD);
}


bool is_lambda_node(GNode *node) {
  return !strcmp(((unitp_t)node->data)->token.str, LAMBDA_KEYWORD);
}
bool is_association(struct Unit *c)
{
  return !strcmp(c->token.str, ASSOCIATION_KEYWORD);
}
bool is_association3(GSList *link) {
  return !strcmp(((unitp_t)link->data)->token.str, ASSOCIATION_KEYWORD);
}
bool is_association4(struct Unit *u) {
  return !strcmp(u->token.str, ASSOCIATION_KEYWORD);
}


bool is_assignment3(GSList *link) {
  return !strcmp(((unitp_t)link->data)->token.str, ASSIGNMENT_KEYWORD);
}
bool is_assignment4(struct Unit *u) {
  return !strcmp(u->token.str, ASSIGNMENT_KEYWORD);
}

bool maybe_binding3(GSList *link)
{
  return unit_type((unitp_t)link->data) == SYMBOL && *((unitp_t)link->data)->token.str == BINDING_PREFIX;
}
bool maybe_binding4(struct Unit *u)
{
  return unit_type(u) == SYMBOL && *u->token.str == BINDING_PREFIX;
}

bool is_binding3(GSList *unit, GSList *parent) {
  return maybe_binding3(unit) &&
    (is_lambda3(parent) || is_association3(unit));
}
bool is_binding4(struct Unit *u, GNode *scope) {
  return maybe_binding4(u) &&
    (is_lambda4((unitp_t)scope->data) || is_association4(u));
}
/* all in one is_builtin??? */
bool is_call(GSList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, "call");
}
bool is_call4(struct Unit *u) {
  return !strcmp(u->token.str, "call");
}


bool is_pret(GSList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, "pret");
}
bool is_pret4(struct Unit *u) {
  return !strcmp(u->token.str, "pret");
}


bool need_subtree4(struct Unit *u, GNode *scope) {
  return is_assignment4(u) ||
    is_association4(u) ||
    is_lambda4(u) ||
    is_binding4(u, scope) ||
    is_call4(u) ||
    is_pret4(u) ||
    !strcmp(u->token.str, "block")
    ;
}

/* goes through the atoms, root will be the container with tl_cons ... */
GNode *parse3(GSList *atoms) {
  GNode *root = g_node_new((unitp_t)atoms->data); /* toplevel atom stattdessen */
  struct Unit *effective_binding_unit = NULL;
  GNode *scope;
  GSList *units_onset = atoms;
  atoms = atoms->next;

  while (atoms) {
    /* find out the DIRECT embedding block of the current cell */
    if (maybe_binding3(atoms) && effective_binding_unit && is_enclosed_in4((unitp_t)atoms->data, effective_binding_unit)) { /* so its a lambda parameter */
      ((unitp_t)atoms->data)->type = BINDING;
      ((unitp_t)atoms->data)->is_atomic = false;
      scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, effective_binding_unit);
      effective_binding_unit->arity++;
      
      /* enhance the type of the parameter symbol. */
      /* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier,
	 denn is_bound_parameter fragt ab ob das Cell vom Typ SYMBOL
	 ist, was wiederum unten im need_new_cons eine Rolle
	 spielt. Deshalb verschieben wir das Setzen vom Typ von SYMBOL zum
	 BOUND_BINDING auf nach need_new_cons. */
    } else {			/* compute the enclosure anew */
      scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, (unitp_t)find_enclosure_link(atoms, units_onset)->data);
    }

    /* i.e. ist das Ding jetzt der WERT für einen Bound Parameter?
       ich frag hier ob das enclosing Zeug von einem bound param
       ist ...*/
    /* enclosure ist hier der block von bound_param */
    if (((unitp_t)scope->data)->type == BINDING || ((unitp_t)scope->data)->type == BOUND_BINDING) {
      ((unitp_t)scope->data)->type = BOUND_BINDING;
      if (((unitp_t)scope->data)->max_capacity == 1) {
	((unitp_t)scope->data)->max_capacity = 0;
      }
      else {
	scope = scope->parent;
      }
    }
    
    /* If the computed enclosing block is a lambda-parameter and it
       has no more absorption capacity then reset the enclosing block
       to be the enclosing block of the lambda-parameter block
       i.e. the lambda block itself (imply that the current item is
       the return-expression of the lambda-block). If the parameter
       block still has absorption capacity (i.e. it's single
       default-argument) the computed enclosing block is correct, only
       decrement it's absorption capacity. */
    if (need_subtree4((unitp_t)atoms->data, scope) || ((unitp_t)atoms->data)->type ==BINDING) {
      ((unitp_t)atoms->data)->is_atomic = false;
      struct Env *env = malloc(sizeof (struct Env));
      *env = (struct Env){
	.enclosing_env = ((unitp_t)scope->data)->env,
	.hash_table = g_hash_table_new(g_str_hash, g_str_equal)
      };
      ((unitp_t)atoms->data)->env = env;
      if (is_lambda3(atoms)) {
	((unitp_t)atoms->data)->arity = 0;
	effective_binding_unit = (unitp_t)atoms->data;
      } else {
	((unitp_t)atoms->data)->arity = -1; /* no lambda, no valid arity! */
      }
      if (is_association4((unitp_t)atoms->data)) {
	effective_binding_unit = (unitp_t)atoms->data;
      }
      if (is_binding4((unitp_t)atoms->data, scope) || ((unitp_t)atoms->data)->type == BINDING) {
	((unitp_t)atoms->data)->max_capacity = 1;
      }
      g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, scope->data),
		    -1, g_node_new((unitp_t)atoms->data));
    } else {			/* it is an atomic unit */
      ((unitp_t)atoms->data)->is_atomic = true;
      ((unitp_t)atoms->data)->max_capacity = 0;
      g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, scope->data),
		    -1,	/* inserted as the last child of parent. */
		    g_node_new((unitp_t)atoms->data));
    }
    atoms = atoms->next;
  }
  return root;
}





void assert_binding_node(GNode *node, GNode *last_child) {
  /* e.g. a parameter or a binding in let */
  if ((node != last_child &&
       !(unit_type((unitp_t)node->data) == BINDING ||
	 unit_type((unitp_t)node->data) == BOUND_BINDING))) {
    fprintf(stderr, "%s not a binding!\n", ((unitp_t)node->data)->token.str);
    exit(EXIT_FAILURE);
  }
}

gboolean ascertain_lambda_spelling(GNode *node, gpointer data) {
  if (is_lambda_node(node)) {
    if (g_node_n_children(node)) {
      GNode *last_child = g_node_last_child(node);
      /* first assert that every child node except with the last one
	 is a binding (ie a parameter decleration) */
      g_node_children_foreach(node, G_TRAVERSE_ALL,
			      (GNodeForeachFunc)assert_binding_node, last_child);
      /* if the last node is a mandatory parameter (i.e. a parameter
	 without default value), it is an error! */
      if (((unitp_t)last_child->data)->type == BINDING) {
	fprintf(stderr, "binding '%s' kann nicht ende von deinem lambda sein\n",
		((unitp_t)last_child->data)->token.str);
	exit(EXIT_FAILURE);
	/* if the last node LOOKS LIKE a parameter with a default
	   argument (i.e. optional argument/bound binding), we see
	   it's default argument as the final expression of lambda */
      } else if (unit_type((unitp_t)last_child->data) == BOUND_BINDING) {
	GNode *bound_value = g_node_last_child(last_child);
	g_node_unlink(bound_value);
	g_node_insert(node, -1, bound_value);
	((unitp_t)last_child->data)->type = BINDING;
      }
    } else {
      fprintf(stderr, "lambda braucht mind. eine expression!\n");
      exit(EXIT_FAILURE);
    }
  }
  return false;
}
void ascertain_lambda_spellings(GNode *root) {
  g_node_traverse(root, G_PRE_ORDER,
		  G_TRAVERSE_ALL, -1,
		  (GNodeTraverseFunc)ascertain_lambda_spelling, NULL);
  puts("Lambda spellings ascertained");
}
