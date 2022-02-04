#include <stdlib.h>		/* EXIT_FAILURE */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "type.h"
#include "token.h"
#include "unit.h"

#include "core.h"

#include "print.h"

#define LEAST_COL_START_IDX -2
#define ASSIGN_MAXCAP 2
#define BIND_MAXCAP 1

bool is_enclosed_in3(GList *ulink1, GList *ulink2) {
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

int bottom_line_number3(GList *list) {
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

GList *find_enclosure_link(GList *unit_link, GList *units_onset) {
  GList *all = enclosures3(unit_link, units_onset);
  GList *undermosts = bottom_enclosures3(all);
  return rightmost_enclosure(undermosts);
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

bool is_association4(struct Unit *u) {
  return !strcmp(u->token.str, ASSOCIATION_KEYWORD);
}


bool is_assignment3(GList *link) {
  return !strcmp(((unitp_t)link->data)->token.str, ASSIGNMENT_KEYWORD);
}
bool is_assignment4(struct Unit *u)
{
  return !strcmp(u->token.str, ASSIGNMENT_KEYWORD);
}
bool is_funcall(struct Unit *u) {
  return !strcmp(u->token.str, FUNCALL_KEYWORD);
}

bool is_cpack(struct Unit *u)
{
  return !strcmp(u->token.str, CPACK_KW);
}

bool maybe_binding3(GList *link)
{
  /* return unit_type((unitp_t)link->data) == NAME && *((unitp_t)link->data)->token.str == BINDING_PREFIX; */
  char *str = ((unitp_t)link->data)->token.str;
  return unit_type((unitp_t)link->data) == NAME && str[strlen(str) - 1] == BINDING_SUFFIX;
}

bool maybe_binding4(struct Unit *u)
{
  /* return unit_type(u) == NAME && *u->token.str == BINDING_PREFIX; */
  char *str = u->token.str;
  return unit_type(u) == NAME && str[strlen(str) - 1] == BINDING_SUFFIX;
}
bool maybe_pack_binding(GList *link)
{
  struct Unit *u = (unitp_t)link->data;
  char *str = u->token.str;
  return unit_type(u) == NAME && /* not a number */
    str[strlen(str) - 1] == BINDING_SUFFIX &&
    *str == PACK_BINDING_PREFIX;
}

bool is_binding4(struct Unit *u, GNode *scope) {
  return maybe_binding4(u) &&
    (is_lambda4((unitp_t)scope->data) ||
     is_association4(u) ||	/* ????? */
     is_association4((unitp_t)scope->data) ||
     is_funcall((unitp_t)scope->data));
}


bool is_pret(GList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, "pret");
}
bool is_pret4(struct Unit *u) {
  return !strcmp(u->token.str, "pret");
}

bool need_block(struct Unit *u, GNode *scope) {
  return is_assignment4(u) ||
    is_association4(u) ||
    is_lambda4(u) ||
    is_binding4(u, scope) ||
    is_pret4(u) ||
    is_funcall(u) ||
    is_cpack(u)
    ;
}

/* statt scope das jetzige atom selbst */
GNode *find_parent_with_capa(GNode *scope) {
  while (scope) {
    if (((unitp_t)scope->data)->max_capacity)
      return scope;
    else
      scope = scope->parent;
  }
  return NULL;		/* nothing found! */
}

struct Unit *find_prev_binding_unit(GList *unit_link) {
  while (unit_link) {
    if ((is_lambda4((unitp_t)unit_link->data) ||
	 is_association4((unitp_t)unit_link->data) ||
	 is_funcall((unitp_t)unit_link->data)) &&
	((unitp_t)unit_link->data)->max_capacity)
      return (unitp_t)unit_link->data;
    else unit_link = unit_link->prev;
  }
  return NULL;
}


/* goes through the atoms, root will be the container with tl_cons ... */
GNode *parse3(GList *unit_link) {
  GNode *root = g_node_new((unitp_t)unit_link->data); /* toplevel atom stattdessen */
  /* effective binding units are units which introduce bindings,
     e.g. lambda, let, pass */
  struct Unit *current_binding_unit = NULL;
  GNode *scope;
  GList *units_onset = unit_link;
  unit_link = unit_link->next;
  GNode *node;
  while (unit_link) {
    /* find out the DIRECT embedding block of the current cell */
    /* Scope */
    if ((maybe_binding3(unit_link) || maybe_pack_binding(unit_link)) &&
	current_binding_unit &&
	is_enclosed_in4((unitp_t)unit_link->data, current_binding_unit)) {
      /* the scope will be the current binding unit */
      scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, current_binding_unit);
      /* funcalls have no arity! */
      if (!is_funcall(current_binding_unit))
	current_binding_unit->arity++;
      
      /* enhance the type of the parameter symbol. */
      /* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier,
	 denn is_bound_parameter fragt ab ob das Cell vom Typ NAME
	 ist, was wiederum unten im need_new_cons eine Rolle
	 spielt. Deshalb verschieben wir das Setzen vom Typ von NAME zum
	 BOUND_BINDING auf nach need_new_cons. */
    } else {			/* compute the enclosure anew */
      scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, (unitp_t)find_enclosure_link(unit_link, units_onset)->data);
    }
    
    /* make a binding based on scope, oben ist nur EBU bedingt! */
    if (is_association4((unitp_t)scope->data) ||
        is_lambda4((unitp_t)scope->data) ||
        is_funcall((unitp_t)scope->data)) {
      if (maybe_pack_binding(unit_link)) /* check pack binding before binding!!! every pack binding is also a binding */
        ((unitp_t)unit_link->data)->type = PACK_BINDING;
      else if (maybe_binding3(unit_link))
        ((unitp_t)unit_link->data)->type = BINDING;
      ((unitp_t)unit_link->data)->is_atomic = false;
    }

    /* i.e. ist das Ding jetzt der WERT für einen Bound Parameter?
       ich frag hier ob das enclosing Zeug von einem bound param
       ist ...*/
    /* enclosure ist hier der block von bound_param */
    if (((unitp_t)scope->data)->type == BINDING || ((unitp_t)scope->data)->type == BOUND_BINDING) {
      ((unitp_t)scope->data)->type = BOUND_BINDING;
      if (((unitp_t)scope->data)->max_capacity == 1) { /* binding has max capa 1 */
	((unitp_t)scope->data)->max_capacity = 0;
      } else {
	scope = find_parent_with_capa(scope);
      }
    } else if (is_assignment4((unitp_t)scope->data)) { /* assign has max_capa 2 */
      if (((unitp_t)scope->data)->max_capacity)
	((unitp_t)scope->data)->max_capacity--;
      else
	scope = find_parent_with_capa(scope);
    } else if (unit_type((unitp_t)scope->data) == PACK_BINDING) {
      ((unitp_t)scope->data)->type = BOUND_PACK_BINDING;
    }

    /* remove this shit later please! */
    if (is_pret4((unitp_t)scope->data)) {
      if (((unitp_t)scope->data)->max_capacity == 1) {
	((unitp_t)scope->data)->max_capacity = 0;
      }
      else {
	scope = find_parent_with_capa(scope);
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
    if (need_block((unitp_t)unit_link->data, scope) ||
        ((unitp_t)unit_link->data)->type == BINDING ||
        unit_type((unitp_t)unit_link->data) == PACK_BINDING) {
      ((unitp_t)unit_link->data)->is_atomic = false;
      ((unitp_t)unit_link->data)->env = g_hash_table_new(g_str_hash, g_str_equal);

      /* set the effective binding unit anew */
      if (is_lambda4((unitp_t)unit_link->data) ||
	  is_funcall((unitp_t)unit_link->data) ||
	  is_association4((unitp_t)unit_link->data))
	current_binding_unit = (unitp_t)unit_link->data;

      /* set the arity for lambda */
      if (is_lambda4((unitp_t)unit_link->data))
	/* we know at this point not much about the number of
	   parameters of this lambda, so set it to 0 (default arity
	   for lambda is 0 parameters). this can change as we go on
	   with parsing and detect it's parameter declerations. */
	((unitp_t)unit_link->data)->arity = 0;

      
      if (is_pret4((unitp_t)unit_link->data)) {
	((unitp_t)unit_link->data)->max_capacity = 1;
	((unitp_t)unit_link->data)->arity = 1;
      }
      /* set the maximum absorption capacity */
      if (is_binding4((unitp_t)unit_link->data, scope) ||
	  ((unitp_t)unit_link->data)->type == BINDING) {
	((unitp_t)unit_link->data)->max_capacity = BIND_MAXCAP;
      } else if (is_assignment4((unitp_t)unit_link->data)) {
	((unitp_t)unit_link->data)->max_capacity = ASSIGN_MAXCAP;
      }
      node = g_node_new((unitp_t)unit_link->data);
      g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, scope->data), -1, node);
    } else {			/* it is an atomic unit */
      ((unitp_t)unit_link->data)->is_atomic = true;
      ((unitp_t)unit_link->data)->max_capacity = 0;
      node = g_node_new((unitp_t)unit_link->data);
      g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, scope->data), -1, node);
    }
    /* when the unit's scope is a lambda/let, the unit is the first item
       of lambda and not a binding form, the unit is the final
       expression of lambda (i.e. lambda will be closed!) */
    if (((is_lambda4((unitp_t)scope->data) ||
	  is_association4((unitp_t)scope->data)) &&
	 unit_type((unitp_t)unit_link->data) != BINDING &&
	 unit_type((unitp_t)unit_link->data) != BOUND_BINDING &&
         unit_type((unitp_t)unit_link->data) != PACK_BINDING &&
         unit_type((unitp_t)unit_link->data) != BOUND_PACK_BINDING &&
	 /* is the first element? */
	 g_node_child_index(scope, (unitp_t)unit_link->data) == 0)) {
      ((unitp_t)scope->data)->max_capacity = 0;
      /* current_binding_unit = find_prev_binding_unit(unit_link->prev); */
      current_binding_unit=NULL;
    }
    /* process next unit */
    unit_link = unit_link->next;
  }
  return root;
}





void assert_lambda_param(GNode *node, GNode *last_child)
{
  if ((node != last_child &&
       unit_type((unitp_t)node->data) != BINDING &&
       unit_type((unitp_t)node->data) != BOUND_BINDING &&
       unit_type((unitp_t)node->data) != PACK_BINDING &&
       unit_type((unitp_t)node->data) != BOUND_PACK_BINDING)) {
    fprintf(stderr, "invalid expression in place of binding\n");
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
			      (GNodeForeachFunc)assert_lambda_param,
			      last_child);
      /* if the last node is a mandatory parameter (i.e. a parameter
	 without default value which i can take out as the lambda
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

void sanify_lambdas(GNode *root) {
  puts("parse-time lambda sanify");
  g_node_traverse(root, G_PRE_ORDER,
		  G_TRAVERSE_ALL, -1,
		  (GNodeTraverseFunc)sanify_lambda, NULL);
  puts("  lambdas sanified");
}


/* schmelz die 2 zusammen mit der obigen */
void assert_bound_binding_node(GNode *node, GNode *last_child) {
  /* e.g. a parameter or a binding in let */
  if ((node != last_child && unit_type((unitp_t)node->data) != BOUND_BINDING)) {
    fprintf(stderr, "malformed binding\n");
    print_node(node, NULL);
    fprintf(stderr, "not bound\n");
    exit(EXIT_FAILURE);
  }
}
void assert_pass_binding(GNode *binding, GNode *lambda) {
  if ((binding != lambda && unit_type((unitp_t)binding->data) == BINDING)) {
    fprintf(stderr, "malformed argument passed\n");
    print_node(binding, NULL);
    fprintf(stderr, "is not bound\n");
    exit(EXIT_FAILURE);
  }
}

gboolean check_funcall(GNode *node, gpointer data) {
  if (is_funcall((unitp_t)node->data)) {
    if (g_node_n_children(node)) {
      GNode *lambda_node = g_node_last_child(node);
      g_node_children_foreach(node, G_TRAVERSE_ALL,
			      (GNodeForeachFunc)assert_pass_binding, lambda_node);
      if (unit_type((unitp_t)lambda_node->data) != NAME) {
	fprintf(stderr, "malformed pass\n");
	print_node(lambda_node, NULL);
	fprintf(stderr, "is not a lambda\n");
	exit(EXIT_FAILURE);
      }
    } else {
      fprintf(stderr, "pass braucht mind. eine expression: func!\n");
      exit(EXIT_FAILURE);
    }
  }
  return false;
}

void check_funcalls(GNode *root) {
  puts("parse-time pass check");
  g_node_traverse(root, G_PRE_ORDER,
		  G_TRAVERSE_ALL, -1,
		  (GNodeTraverseFunc)check_funcall, NULL);
  puts("  pass checked");
}
gboolean check_assoc(GNode *node, gpointer data) {
  if (is_association4((unitp_t)node->data)) {
    if (g_node_n_children(node)) {
      GNode *last_child = g_node_last_child(node);
      /* first assert that every child node except with the last one
	 is a bound binding */
      g_node_children_foreach(node, G_TRAVERSE_ALL,
			      (GNodeForeachFunc)assert_bound_binding_node,
			      last_child);
      if (unit_type((unitp_t)last_child->data) == BINDING) {
	fprintf(stderr, "malformed association\n");
	print_node(last_child, NULL);
	fprintf(stderr, "not bound\n");
	exit(EXIT_FAILURE);
      } else if (unit_type((unitp_t)last_child->data) == BOUND_BINDING) {
	fprintf(stderr, "malformed association\n");
	print_node(node, NULL);
	fprintf(stderr, "expression missing\n");
	exit(EXIT_FAILURE);
      }
    } else {
      fprintf(stderr, "malformed association\n");
      print_node(node, NULL);
      fprintf(stderr, "missing expression\n");
      exit(EXIT_FAILURE);
    }
  }
  return false;
}

void check_assocs(GNode *root) {
  puts("parse-time assocs check");
  g_node_traverse(root, G_PRE_ORDER,
		  G_TRAVERSE_ALL, -1,
		  (GNodeTraverseFunc)check_assoc, NULL);
  puts("  assocs checked");
}
