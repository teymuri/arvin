#include <stdlib.h>		/* EXIT_FAILURE */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "type.h"
#include "env.h"
#include "token.h"
#include "atom.h"
#include "cons.h"
#include "core.h"
#include "cons_elem.h"
#include "print.h"

#define LEAST_COL_START_IDX -2

/* /\* is this unit directly/indirectly enclosed in this construct? *\/ */
/* bool is_enclosed_in(struct Atom c, struct Cons b) */
/* { */
/*   return (c.token.col_start_idx > b.bricks[0]->token.col_start_idx) && */
/*     (c.token.line >= b.bricks[0]->token.line); */
/* } */

bool is_enclosed_in3(GSList *ulink1, GSList *ulink2) {
  if (ulink2 != NULL) {
    struct Atom *u1 = (unitp_t)ulink1->data, *u2 = (unitp_t)ulink2->data;
    return u1->token.col_start_idx > u2->token.col_start_idx &&
      u1->token.line >= u2->token.line;
  } else {
    return false;
  }
}
bool is_enclosed_in4(struct Atom *a1, struct Atom *a2) {
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
    if (!((unitp_t)list->data)->is_infertile && is_enclosed_in3(ulink, list))
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

/* /\* all blocks in which the cell is embedded. returns a pointer which */
/*    points to pointers to block structures, so it's return value must */
/*    be freed (which doesn't any harm to the actual structure pointers */
/*    it points to!) *\/ */
/* struct Cons **enclosing_blocks__Hp(struct Atom c, struct Cons **blocks, */
/* 				    int blocks_count, int *enblocks_count) */
/* { */
/*   struct Cons **enblocks = NULL; */
/*   for (int i = 0; i < blocks_count; i++) { */
/*     if (is_enclosed_in(c, *(blocks[i]))) { */
/*       if ((enblocks = realloc(enblocks, (*enblocks_count + 1) * sizeof(struct Cons *))) != NULL) */
/* 	*(enblocks + (*enblocks_count)++) = *(blocks + i); */
/*       else exit(EXIT_FAILURE); */
/*     } */
/*   } */
/*   return enblocks; */
/* } */







/* /\* returns the bottom line number *\/ */
/* int bottom_line_number(struct Cons **enblocks, int enblocks_count) */
/* { */
/*   int ln = -1; */
/*   for (int i = 0; i < enblocks_count; i++) { */
/*     if ((*(enblocks + i))->bricks[0]->token.line > ln) */
/*       ln = (*(enblocks + i))->bricks[0]->token.line; */
/*   } */
/*   return ln; */
/* } */

/* struct Cons **bottommost_blocks__Hp(struct Cons **enblocks, int enblocks_count, int *botmost_blocks_count) */
/* { */
/*   int bln = bottom_line_number(enblocks, enblocks_count); */
/*   struct Cons **botmost_blocks = NULL; */
/*   for (int i = 0; i < enblocks_count; i++) { */
/*     if ((*(enblocks + i))->bricks[0]->token.line == bln) { */
/*       if ((botmost_blocks = realloc(botmost_blocks, (*botmost_blocks_count + 1) * sizeof(struct Cons *))) != NULL) { */
/* 	*(botmost_blocks + (*botmost_blocks_count)++) = *(enblocks + i); */
/*       }	else exit(EXIT_FAILURE); */
/*     } */
/*   } */
/*   /\* free the pointer to selected (i.e. embedding) block pointers *\/ */
/*   free(enblocks); */
/*   return botmost_blocks; */
/* } */




/* /\* here we test column start index of block heads to decide *\/ */
/* struct Cons *rightmost_block(struct Cons **botmost_blocks, int botmost_blocks_count) */
/* { */
/*   int col_start_idx = LEAST_COL_START_IDX;			/\* start index *\/ */
/*   struct Cons *rmost_block = NULL; */
/*   for (int i = 0; i < botmost_blocks_count; i++) {     */
/*     if ((*(botmost_blocks + i))->bricks[0]->token.col_start_idx > col_start_idx) { */
/*       rmost_block = *(botmost_blocks + i); */
/*       col_start_idx = rmost_block->bricks[0]->token.col_start_idx; */
/*     } */
/*   } */
/*   free(botmost_blocks); */
/*   return rmost_block; */
/* } */


/* /\* which one of the blocks is the direct embedding block of c? *\/ */
/* struct Cons *enclosing_block(struct Atom c, struct Cons **blocks, int blocks_count) */
/* {   */
/*   int enblocks_count = 0; */
/*   struct Cons **enblocks = enclosing_blocks__Hp(c, blocks, blocks_count, &enblocks_count); */
/*   int botmost_blocks_count = 0; */
/*   struct Cons **botmost_blocks = bottommost_blocks__Hp(enblocks, enblocks_count, &botmost_blocks_count); */
/*   return rightmost_block(botmost_blocks, botmost_blocks_count); */
/* } */



/* whether string s1 ends with string s2? */
int str_ends_with(char *str1, char *str2)
{
  size_t len1 = strlen(str1);
  size_t len2 = strlen(str2);
  for (size_t i = 0; i<len2;i++) {
    if (!(str1[len1-(len2-i)] == str2[i])) {
      return 0;
    }
  }
  return 1;
}

/* bool looks_like_parameter(struct Atom *c) */
/* { */
/*   return atom_type(c) == SYMBOL && str_ends_with(c->token.str, ":"); */
/* } */
/* bool looks_like_bound_parameter(struct Atom *c) */
/* { */
/*   return atom_type(c) == SYMBOL && str_ends_with(c->token.str, ":="); */
/* } */


/* to be included also in eval */
bool is_lambda_unit(struct Atom *u)
{
  return !strcmp(u->token.str, LAMBDA_KW);
}

bool is_lambda_head(struct Atom c) { return !strcmp(c.token.str, LAMBDA_KW); }
bool is_lambda3(GSList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, LAMBDA_KW);
}


bool is_lambda4(struct Atom *u) {
  return !strcmp(u->token.str, LAMBDA_KW);
}


bool is_lambda_node(GNode *node) {
  return !strcmp(((unitp_t)node->data)->token.str, LAMBDA_KW);
}
bool is_association(struct Atom *c)
{
  return !strcmp(c->token.str, ASSOCIATION_KEYWORD);
}
bool is_association3(GSList *link) {
  return !strcmp(((unitp_t)link->data)->token.str, ASSOCIATION_KEYWORD);
}
bool is_association4(struct Atom *u) {
  return !strcmp(u->token.str, ASSOCIATION_KEYWORD);
}


bool is_assignment3(GSList *link) {
  return !strcmp(((unitp_t)link->data)->token.str, ASSIGNMENT_KEYWORD);
}
bool is_assignment4(struct Atom *u) {
  return !strcmp(u->token.str, ASSIGNMENT_KEYWORD);
}

/* now we will be sure! */
/* bool is_parameter(struct Atom *c, struct Cons *enclosing_block) */
/* { */
/*   return looks_like_parameter(c) */
/*     && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).token.str, */
/* 						       ASSOCIATION_KEYWORD)); */
/* } */


/* bool maybe_binding(struct Atom *b) */
/* { */
/*   return atom_type(a) == SYMBOL && *a->token.str == '.'; */
/* } */
bool maybe_binding3(GSList *link)
{
  return atom_type((unitp_t)link->data) == SYMBOL && *((unitp_t)link->data)->token.str == '.';
}
bool maybe_binding4(struct Atom *u)
{
  return atom_type(u) == SYMBOL && *u->token.str == '.';
}


/* bool is_binding(struct Atom *b, struct Cons *enclosure) */
/* { */
/*   return maybe_binding(b) && */
/*     (is_lambda_head(block_head(enclosure)) || */
/*      !strcmp((block_head(enclosure)).token.str, ASSOCIATION_KEYWORD)); */
/* } */
bool is_binding3(GSList *unit, GSList *parent) {
  return maybe_binding3(unit) &&
    (is_lambda3(parent) || is_association3(unit));
}
bool is_binding4(struct Atom *u, GNode *scope) {
  return maybe_binding4(u) &&
    (is_lambda4((unitp_t)scope->data) || is_association4(u));
}

/* bool is_bound_binding(struct Atom *c) */
/* { */
/*   return c->type == BOUND_BINDING; */
/* } */

/* bool is_bound_parameter(struct Atom *c, struct Cons *enclosing_block) */
/* { */
/*   return looks_like_bound_parameter(c) */
/*     && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).token.str, */
/* 						       ASSOCIATION_KEYWORD)); */
/* } */


/* is the direct enclosing block the bind keyword, ie we are about to
   define a new name? */
/* bool is_a_binding_name(struct Cons *b) */
/* { */
/*   return !strcmp(block_head(b->enclosure).token.str, ASSIGNMENT_KEYWORD); */
/* } */


/* bool need_new_cons(struct Atom *c, struct Cons *enclosing_block) */
/* { */
/*   return isbuiltin(c) */
/*     || !strcmp(c->token.str, ASSIGNMENT_KEYWORD) */
/*     || is_association(c) */
/*     /\* is the symbol to be defined? *\/ */
/*     /\* || !strcmp(block_head(enclosing_block).token.str, ASSIGNMENT_KEYWORD) *\/ */
/*     /\* is begin of a lambda expression? *\/ */
/*     || is_lambda_head(*c) */
/*     /\* is a lambda parameter? *\/ */
/*     || is_binding(c, enclosing_block) /\* hier muss enclosing_block richtig entschieden sein!!! *\/ */
/*     /\* || is_bound_parameter(c, enclosing_block) /\\* hier muss enclosing_block richtig entschieden sein!!! *\\/ *\/ */
/*     || !strcmp(c->token.str, "call") */
/*     || !strcmp(c->token.str, "pret") */
/*     || !strcmp(c->token.str, "gj") /\* geburtsjahr!!! *\/ */
/*     ; */
/* } */
/* all in one is_builtin??? */
bool is_call(GSList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, "call");
}
bool is_call4(struct Atom *u) {
  return !strcmp(u->token.str, "call");
}


bool is_pret(GSList *unit) {
  return !strcmp(((unitp_t)unit->data)->token.str, "pret");
}
bool is_pret4(struct Atom *u) {
  return !strcmp(u->token.str, "pret");
}

bool need_subtree(GSList *unit, GSList *parent) {
  return is_assignment3(unit) ||
    is_association3(unit) ||
    is_lambda3(unit) ||
    is_binding3(unit, parent) ||
    is_call(unit) ||
    is_pret(unit)
    ;
}

bool need_subtree4(struct Atom *u, GNode *scope) {
  return is_assignment4(u) ||
    is_association4(u) ||
    is_lambda4(u) ||
    is_binding4(u, scope) ||
    is_call4(u) ||
    is_pret4(u)
    ;
}

/* struct Cons **parse__Hp(struct Cons *global_block, struct Atom *linked_cells_root, int *blocks_count) */
/* { */
/*   /\* this is the blocktracker in the python prototype *\/ */
/*   struct Cons **blocks = malloc(sizeof (struct Cons *)); /\* make room for the toplevel block *\/ */
/*   *(blocks + (*blocks_count)++) = global_block; */
/*   struct Atom *c = linked_cells_root; */
/*   struct Cons *enclosure;	/\* the enclosing bundle *\/ */
/*   struct Cons *active_binding_plate; /\* this is the last lambda, let etc. *\/ */
/*   int blockid = 1; */
/*   while (c) { */
    
/*     /\* find out the DIRECT embedding block of the current cell *\/ */
/*     /\* (looks_like_parameter(c) || looks_like_bound_parameter(c)) *\/ */
/*     if (maybe_binding(c) && is_enclosed_in(*c, *active_binding_plate)) { /\* so its a lambda parameter *\/ */
/*       c->type = BINDING; */
/*       enclosure = active_binding_plate; */
/*       active_binding_plate->arity++; */
/*       /\* enhance the type of the parameter symbol. *\/ */
/*       /\* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier, */
/* 	 denn is_bound_parameter fragt ab ob das Cell vom Typ SYMBOL */
/* 	 ist, was wiederum unten im need_new_cons eine Rolle */
/* 	 spielt. Deshalb verschieben wir das Setzen vom Typ von SYMBOL zum */
/* 	 BOUND_BINDING auf nach need_new_cons. *\/ */
/*       /\* if (is_parameter(c, enclosure)) c->type = BINDING; *\/ */
/*       /\* if (is_bound_parameter(c, enclosure)) c->type = BOUND_BINDING; *\/ */
/*     } else {			/\* compute the enclosure anew *\/ */
/*       enclosure = enclosing_block(*c, blocks, *blocks_count); */
/*     } */

/*     /\* i.e. ist das Ding jetzt der WERT für einen Bound Parameter? */
/*        ich frag hier ob das enclosing Zeug von einem bound param */
/*        ist ...*\/ */
/*     /\* enclosure ist hier der block von bound_param *\/ */
/*     /\* is_binding(enclosure->bricks + 0, enclosure->enclosure) *\/ */
/*     if ((*enclosure->bricks)->type == BINDING || (*enclosure->bricks)->type == BOUND_BINDING) { */
/*       (*enclosure->bricks)->type = BOUND_BINDING; */
/*       if (enclosure->max_absr_capa == 1) */
/* 	enclosure->max_absr_capa = 0; */
/*       else { */
/* 	enclosure = enclosure->enclosure; */
/*       } */
/*     } */
/*     /\* enclosure->enclosure->bricks->token.str; *\/ */
    
/*     /\* If the computed enclosing block is a lambda-parameter and it */
/*        has no more absorption capacity then reset the enclosing block */
/*        to be the enclosing block of the lambda-parameter block */
/*        i.e. the lambda block itself (imply that the current item is */
/*        the return-expression of the lambda-block). If the parameter */
/*        block still has absorption capacity (i.e. it's single */
/*        default-argument) the computed enclosing block is correct, only */
/*        decrement it's absorption capacity. *\/ */
/*     /\* &(enclosing_block->bricks[0]) *\/ */
    
/*     if (need_new_cons(c, enclosure) || c->type==BINDING) { */
/*       if ((blocks = realloc(blocks, (*blocks_count + 1) * sizeof (struct Cons *))) != NULL) { */

/* 	struct Cons *newplt = malloc(sizeof *newplt); */
/* 	newplt->bricks=malloc(sizeof (struct Atom *)); */
/* 	struct Env *newenv = malloc(sizeof *newenv); */
/* 	newplt->id = blockid++; */
/* 	*newplt->bricks = c; */
/* 	newplt->size = 1; */
/* 	newplt->enclosure = enclosure; */
/* 	*newenv = (struct Env){ */
/* 	  .enclosing_env = enclosure->env, */
/* 	  .hash_table = g_hash_table_new(g_str_hash, g_str_equal) */
/* 	}; */
/* 	newplt->env = newenv; */

/* 	/\* set the new block's content *\/ */
/* 	newplt->elts = malloc(sizeof (struct Cons_item)); */
/* 	(*(newplt->elts)).type = ATOM; */
/* 	(*(newplt->elts)).the_unit = c; */
	
/* 	*(blocks + (*blocks_count)++) = newplt; */
	
/* 	/\* keep an eye on this if its THE BEGINNING of a lambda *\/ */
/* 	if (is_lambda_head(*c)) { */
/* 	  newplt->islambda = true; /\* is a lambda-block *\/ */
/* 	  newplt->arity = 0; /\* default is null-arity *\/ */
/* 	  active_binding_plate = newplt; */
/* 	  c->type=LAMBDA; */
/* 	} else { */
/* 	  newplt->islambda = false; */
/* 	} */
/* 	/\* LET Block *\/ */
/* 	if (is_association(c)) { */
/* 	  active_binding_plate = newplt; */
/* 	} */

/* 	if (is_binding(c, enclosure) || c->type==BINDING) { */
/* 	  /\* printf("Christoph Seibert"); *\/ */
/* 	  newplt->max_absr_capa = 1;	/\* ist maximal das default argument wenn vorhanden *\/ */
/* 	  /\* enhance the type from simple SYMBOL to BOUND_BINDING *\/ */
/* 	  /\* c->type = BOUND_BINDING; *\/ */
/* 	} */
		
/* 	/\* das ist doppel gemoppelt, fass die beiden unten zusammen... *\/ */
/* 	if ((enclosure->elts = realloc(enclosure->elts, (enclosure->size+1) * sizeof(struct Cons_item))) != NULL) { */
/* 	  (*(enclosure->elts + enclosure->size)).type = CONS; */
/* 	  (*(enclosure->elts + enclosure->size)).the_const = newplt; */
/* 	  enclosure->size++; */
/* 	} */
/*       } else exit(EXIT_FAILURE); /\* blocks realloc failed *\/       */
/*     } else {			 /\* no need for a new block, just a single lonely cell *\/ */
/*       if ((enclosure->elts = realloc(enclosure->elts, (enclosure->size+1) * sizeof(struct Cons_item))) != NULL) { */
/* 	(*(enclosure->elts + enclosure->size)).type = ATOM; */
/* 	(*(enclosure->elts + enclosure->size)).the_unit = c; */
/*       } */
/*       c->enclosure = enclosure; */
/*       /\* enclosure->bricks[enclosure->size] = *c; *\/ */
/*       if ((enclosure->bricks = realloc(enclosure->bricks, (enclosure->size + 1) * sizeof (struct Atom *))) != NULL) { */
/* 	*(enclosure->bricks + enclosure->size) = c; */
/*       } */
/*       enclosure->size++; */
/*     } */
/*     c = c->next; */
/*   } */
/*   return blocks; */
/* } */

/* goes through the atoms, root will be the container with tl_cons ... */
GNode *parse3(GSList *atoms)
{
  GNode *root = g_node_new((unitp_t)atoms->data); /* toplevel atom stattdessen */
  /* this is the blocktracker in the python prototype */
  /* struct Cons **blocks = malloc(sizeof (struct Cons *)); /\* make room for the toplevel block *\/ */
  /* *(blocks + (*blocks_count)++) = tl_cons; */
  /* struct Atom *c = linked_cells_root; */
  /* GSList *enclosure = NULL;	/\* the enclosing bundle *\/ */
  struct Atom *effective_binding_unit = NULL;
  GNode *scope;
  /* GSList *active_binding_plate = NULL; /\* this is the last lambda, let etc. *\/ */
  GSList *units_onset = atoms;
  atoms = atoms->next;

  while (atoms) {
    /* struct Atom *atom = atoms->data; */
    /* find out the DIRECT embedding block of the current cell */
    /* (looks_like_parameter(c) || looks_like_bound_parameter(c)) */
    if (maybe_binding3(atoms) && effective_binding_unit && is_enclosed_in4((unitp_t)atoms->data, effective_binding_unit)) { /* so its a lambda parameter */
      ((unitp_t)atoms->data)->type = BINDING;
      ((unitp_t)atoms->data)->is_infertile = false;
      /* enclosure = active_binding_plate; */
      scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, effective_binding_unit);
      /* printf("%s %s %d %d\n", ((unitp_t)atoms->data)->token.str, */
      /* 	     stringify_type(((unitp_t)atoms->data)->type), */
      /* 	     ); */
	    
      /* ((unitp_t)active_binding_plate->data)->arity++; */
      effective_binding_unit->arity++;
      
      /* enhance the type of the parameter symbol. */
      /* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier,
	 denn is_bound_parameter fragt ab ob das Cell vom Typ SYMBOL
	 ist, was wiederum unten im need_new_cons eine Rolle
	 spielt. Deshalb verschieben wir das Setzen vom Typ von SYMBOL zum
	 BOUND_BINDING auf nach need_new_cons. */
      /* if (is_parameter(c, enclosure)) c->type = BINDING; */
      /* if (is_bound_parameter(c, enclosure)) c->type = BOUND_BINDING; */
    } else {			/* compute the enclosure anew */
      /* enclosure = find_enclosure_link(atoms, units_onset); */
      scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, (unitp_t)find_enclosure_link(atoms, units_onset)->data);
    }

    /* i.e. ist das Ding jetzt der WERT für einen Bound Parameter?
       ich frag hier ob das enclosing Zeug von einem bound param
       ist ...*/
    /* enclosure ist hier der block von bound_param */
    /* is_binding(enclosure->bricks + 0, enclosure->enclosure) */

    if (((unitp_t)scope->data)->type == BINDING || ((unitp_t)scope->data)->type == BOUND_BINDING) {
      ((unitp_t)scope->data)->type = BOUND_BINDING;
      if (((unitp_t)scope->data)->max_absorption_capacity == 1) {
	((unitp_t)scope->data)->max_absorption_capacity = 0;
      }
      else {
	/* enclosure=g_slist_find(units_onset, ((unitp_t)enclosure->data)->parent_unit); */
	/* scope = g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, (unitp_t)enclosure->data); */
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
    /* &(enclosing_block->bricks[0]) */
    /* need_subtree(atoms, scope) */

    if (need_subtree4((unitp_t)atoms->data, scope) || ((unitp_t)atoms->data)->type ==BINDING) {
      ((unitp_t)atoms->data)->is_infertile = false;
      struct Env *env = malloc(sizeof (struct Env));
      *env = (struct Env){
	.enclosing_env = ((unitp_t)scope->data)->env,
	.hash_table = g_hash_table_new(g_str_hash, g_str_equal)
      };
      ((unitp_t)atoms->data)->env = env;
      if (is_lambda3(atoms)) {
	((unitp_t)atoms->data)->arity = 0;
	/* active_binding_plate = atoms; */
	effective_binding_unit = (unitp_t)atoms->data;
      } else {
	((unitp_t)atoms->data)->arity = -1; /* no lambda, no valid arity! */
      }
      if (is_association4((unitp_t)atoms->data)) {
	/* active_binding_plate = atoms; */
	effective_binding_unit = (unitp_t)atoms->data;
      }
      if (is_binding4((unitp_t)atoms->data, scope) || ((unitp_t)atoms->data)->type == BINDING) {
	((unitp_t)atoms->data)->max_absorption_capacity = 1;
      }
      g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, scope->data),
		    -1, g_node_new((unitp_t)atoms->data));
    }
    else
      {
	((unitp_t)atoms->data)->is_infertile = true;
	g_node_insert(g_node_find(root, G_PRE_ORDER, G_TRAVERSE_ALL, scope->data),
		      -1,	/* inserted as the last child of parent. */
		      g_node_new((unitp_t)atoms->data));

	  
      }
    atoms = atoms->next;
  }
  return root;
}




void print_code_ast(struct Cons *root, int depth);
char *stringify_block_item_type(enum Cons_item_type t);

/* append elt as the last item to plt's elts */
void append_element(struct Cons_item *elt, struct Cons *plt)
{  
  /* ******************************* */
  if ((plt->elts = realloc(plt->elts,
			    (plt->size+1) * sizeof (struct Cons_item))) != NULL) {
	(plt->elts + plt->size)->type = elt->type;
	if (elt->type == ATOM) {
	  (plt->elts + plt->size)->the_unit = elt->the_unit;
	  /* auch in bricks */
	  if ((plt->bricks = realloc(plt->bricks, (plt->size+1)*sizeof (struct Atom *))) != NULL)
	    *(plt->bricks + plt->size) = elt->the_unit;
	}
	else			/* hoffentlich CONS!!! */
	  (plt->elts + plt->size)->the_const = elt->the_const;
	plt->size++;
  }
  /* ************************ */
}

/* struct Cons *remove_element(struct Cons_item *elt, struct Cons *plt) */
/* { */
/*   for (int i = 0; i < plt->size; i++) { */
    
/*   } */
/*   return elt; */
/* } */
gboolean ascertain_lambda_spelling(GNode *node, gpointer data) {
  if (is_lambda_node(node)) {
    switch (g_node_n_children(node)) {
    case 0:
      /* error */
      break;
    case 1:
      /* muss die Expression sein, die fassen wir nicht an! */
      /* assert(((struct Atom *)node->children->data)->type != BINDING);  */
      if (((struct Atom *)node->children->data)->type == BINDING) {
	fprintf(stderr, "binding '%s' kann nicht ende von deinem lambda sein\n",
		((struct Atom *)node->children->data)->token.str);
	exit(EXIT_FAILURE);
      }
      break;
    default:
      /* es gibt parameterliste */
      
      break;
    }
  }
  return false;
}
void ascertain_all_lambda_spellings(GNode *root) {
  g_node_traverse(root, G_PRE_ORDER,
		  G_TRAVERSE_ALL, -1,
		  (GNodeTraverseFunc)ascertain_lambda_spelling, NULL);
}
/* testen wir mal die Semantik von Lambda expressions */
void amend_lambda_semantics(struct Cons *root)
{
  for (int i = 0; i < root->size; i++) {
    switch (root->elts[i].type) {
      /* a lambda can only ever be a plate */
    case ATOM: break;
    case CONS:
      if (atom_type(*root->elts[i].the_const->bricks) == LAMBDA) {
	int sz = (*root->elts[i].the_const).size;
	printf("====%p=====\n", (void *)*root->elts[i].the_const->bricks);
	/* for (int j = 1; j < sz-1; j++) { */
	/*   /\* bestimmt plates *\/ */
	/*   printf("->%d %s\n", j,(*root->elts[i].the_const->elts[j].the_const->bricks)->token.str); */
	/* } */
	struct Cons_item *body = root->elts[i].the_const->elts + (sz - 1); /*  */
	/* body teil */
	switch (body->type) {
	case ATOM: break;	/* sieht gut aus (a cell), klassen wir mal so bleiben */
	case CONS:
	  /* printf("??%s\n", stringify_type(atom_type(*body->the_const->bricks))); */
	  if (atom_type(*body->the_const->bricks) == BINDING) {
	    fprintf(stderr, "binding '%s' kann nicht ende von deinem lambda sein\n",
		    (*body->the_const->bricks)->token.str);
	    exit(EXIT_FAILURE);
	  } else if (atom_type(*body->the_const->bricks) == BOUND_BINDING) { /* .x ... */
	    
	    /* (root->elts[i].the_const->size)++; */
	    
	    /* the_const hat genau 2 teile:  */
	    struct Cons_item *retstat = body->the_const->elts + 1; /* ... */
	    /* struct Atom *retstat = body->the_const->bricks + 1; */
	    if (retstat->type ==ATOM) {
	      retstat->the_unit->enclosure = body->the_const->enclosure;
	      /* body->the_const->size--; */
	      body->the_const->elts->the_unit->type = BINDING;
	      printf("retstat %s %s %p %d %s\n", retstat->the_unit->token.str,
		     (*retstat->the_unit->enclosure->bricks)->token.str,
		     (void *)*retstat->the_unit->enclosure->bricks,
		     root->elts[i].the_const->size, (*root->elts[i].the_const->bricks)->token.str
		     );
	      append_element(retstat, body->the_const->enclosure);
	    }
	  }
	  printf("->%s\n", (*root->elts[i].the_const->elts[sz-1].the_const->bricks)->token.str);
	  break;
	}
      }
      amend_lambda_semantics(root->elts[i].the_const);
      break;
    default: break;
    }
  }
}

void free_parser_blocks(struct Cons **blocks, int blocks_count)
{
  /* the first pointer in **blocks points to the global_block, thats why we
     can't free *(blocks + 0), as global_block is created on the
     stack in main(). that first pointer will be freed after the for loop. */
  for (int i = 1; i < blocks_count; i++) {
    free((*(blocks + i))->elts);
    free(*(blocks + i));
  }
  /* free the content of the toplevel block, since it surely
     containts something when the parsed string hasn't been an empty
     string! */
  free((*blocks)->elts);
  free(blocks);
}
