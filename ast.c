#include <stdlib.h>		/* EXIT_FAILURE */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "type.h"
#include "env.h"
#include "token.h"
#include "brick.h"
#include "plate.h"
#include "core.h"
#include "plate_element.h"




/* is b directly or indirectly embedding c? */
bool is_enclosed_in(struct Brick c, struct Plate b)
{
  return (c.token.column_start_idx > b.bricks[0]->token.column_start_idx)
    && (c.token.line >= b.bricks[0]->token.line);
}


/* all blocks in which the cell is embedded. returns a pointer which
   points to pointers to block structures, so it's return value must
   be freed (which doesn't any harm to the actual structure pointers
   it points to!) */
struct Plate **enclosing_blocks__Hp(struct Brick c, struct Plate **blocks,
				    int blocks_count, int *enblocks_count)
{
  struct Plate **enblocks = NULL;
  for (int i = 0; i < blocks_count; i++) {
    if (is_enclosed_in(c, *(blocks[i]))) {
      if ((enblocks = realloc(enblocks, (*enblocks_count + 1) * sizeof(struct Plate *))) != NULL)
	*(enblocks + (*enblocks_count)++) = *(blocks + i);
      else exit(EXIT_FAILURE);
    }
  }
  return enblocks;
}

/* returns the bottom line number */
int bottom_line_number(struct Plate **enblocks, int enblocks_count)
{
  int ln = -1;
  for (int i = 0; i < enblocks_count; i++) {
    if ((*(enblocks + i))->bricks[0]->token.line > ln)
      ln = (*(enblocks + i))->bricks[0]->token.line;
  }
  return ln;
}

struct Plate **bottommost_blocks__Hp(struct Plate **enblocks, int enblocks_count, int *botmost_blocks_count)
{
  int bln = bottom_line_number(enblocks, enblocks_count);
  struct Plate **botmost_blocks = NULL;
  for (int i = 0; i < enblocks_count; i++) {
    if ((*(enblocks + i))->bricks[0]->token.line == bln) {
      if ((botmost_blocks = realloc(botmost_blocks, (*botmost_blocks_count + 1) * sizeof(struct Plate *))) != NULL) {
	*(botmost_blocks + (*botmost_blocks_count)++) = *(enblocks + i);
      }	else exit(EXIT_FAILURE);
    }
  }
  /* free the pointer to selected (i.e. embedding) block pointers */
  free(enblocks);
  return botmost_blocks;
}

#define LEAST_COL_START_IDX -2
/* here we test column start index of block heads to decide */
struct Plate *rightmost_block(struct Plate **botmost_blocks, int botmost_blocks_count)
{
  int column_start_idx = LEAST_COL_START_IDX;			/* start index */
  struct Plate *rmost_block = NULL;
  for (int i = 0; i < botmost_blocks_count; i++) {    
    if ((*(botmost_blocks + i))->bricks[0]->token.column_start_idx > column_start_idx) {
      rmost_block = *(botmost_blocks + i);
      column_start_idx = rmost_block->bricks[0]->token.column_start_idx;
    }
  }
  free(botmost_blocks);
  return rmost_block;
}

/* which one of the blocks is the direct embedding block of c? */
struct Plate *enclosing_block(struct Brick c, struct Plate **blocks, int blocks_count)
{  
  int enblocks_count = 0;
  struct Plate **enblocks = enclosing_blocks__Hp(c, blocks, blocks_count, &enblocks_count);
  int botmost_blocks_count = 0;
  struct Plate **botmost_blocks = bottommost_blocks__Hp(enblocks, enblocks_count, &botmost_blocks_count);
  return rightmost_block(botmost_blocks, botmost_blocks_count);
}



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

/* bool looks_like_parameter(struct Brick *c) */
/* { */
/*   return brick_type(c) == SYMBOL && str_ends_with(c->token.str, ":"); */
/* } */
/* bool looks_like_bound_parameter(struct Brick *c) */
/* { */
/*   return brick_type(c) == SYMBOL && str_ends_with(c->token.str, ":="); */
/* } */


/* to be included also in eval */
bool is_lambda_unit(struct Brick *u)
{
  return !strcmp(u->token.str, LAMBDA_KW);
}

bool is_lambda_head(struct Brick c) { return !strcmp(c.token.str, LAMBDA_KW); }
bool is_association(struct Brick *c)
{
  return !strcmp(c->token.str, ASSOCIATION_KEYWORD);
}

/* now we will be sure! */
/* bool is_parameter(struct Brick *c, struct Plate *enclosing_block) */
/* { */
/*   return looks_like_parameter(c) */
/*     && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).token.str, */
/* 						       ASSOCIATION_KEYWORD)); */
/* } */


bool maybe_binding(struct Brick *b)
{
  return brick_type(b) == SYMBOL && *b->token.str == '.';
}
bool is_binding(struct Brick *b, struct Plate *enclosure)
{
  return maybe_binding(b) &&
    (is_lambda_head(block_head(enclosure)) ||
     !strcmp((block_head(enclosure)).token.str, ASSOCIATION_KEYWORD));
}
bool is_bound_binding(struct Brick *c)
{
  return c->type == BOUND_BINDING;
}

/* bool is_bound_parameter(struct Brick *c, struct Plate *enclosing_block) */
/* { */
/*   return looks_like_bound_parameter(c) */
/*     && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).token.str, */
/* 						       ASSOCIATION_KEYWORD)); */
/* } */


/* is the direct enclosing block the bind keyword, ie we are about to
   define a new name? */
/* bool is_a_binding_name(struct Plate *b) */
/* { */
/*   return !strcmp(block_head(b->plate).token.str, ASSIGNMENT_KEYWORD); */
/* } */


bool need_new_block(struct Brick *c, struct Plate *enclosing_block)
{
  return isbuiltin(c)
    || !strcmp(c->token.str, ASSIGNMENT_KEYWORD)
    || is_association(c)
    /* is the symbol to be defined? */
    /* || !strcmp(block_head(enclosing_block).token.str, ASSIGNMENT_KEYWORD) */
    /* is begin of a lambda expression? */
    || is_lambda_head(*c)
    /* is a lambda parameter? */
    || is_binding(c, enclosing_block) /* hier muss enclosing_block richtig entschieden sein!!! */
    /* || is_bound_parameter(c, enclosing_block) /\* hier muss enclosing_block richtig entschieden sein!!! *\/ */
    || !strcmp(c->token.str, "call")
    || !strcmp(c->token.str, "pret")
    || !strcmp(c->token.str, "gj") /* geburtsjahr!!! */
    ;
}


struct Plate **parse__Hp(struct Plate *global_block, struct Brick *linked_cells_root, int *blocks_count)
{
  /* this is the blocktracker in the python prototype */
  struct Plate **blocks = malloc(sizeof (struct Plate *)); /* make room for the toplevel block */
  *(blocks + (*blocks_count)++) = global_block;
  struct Brick *c = linked_cells_root;
  struct Plate *enclosure;	/* the enclosing bundle */
  struct Plate *active_binding_plate; /* this is the last lambda, let etc. */
  int blockid = 1;
  while (c) {
    
    /* find out the DIRECT embedding block of the current cell */
    /* (looks_like_parameter(c) || looks_like_bound_parameter(c)) */
    if (maybe_binding(c) && is_enclosed_in(*c, *active_binding_plate)) { /* so its a lambda parameter */
      c->type = BINDING;
      enclosure = active_binding_plate;
      active_binding_plate->arity++;
      /* enhance the type of the parameter symbol. */
      /* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier,
	 denn is_bound_parameter fragt ab ob das Cell vom Typ SYMBOL
	 ist, was wiederum unten im need_new_block eine Rolle
	 spielt. Deshalb verschieben wir das Setzen vom Typ von SYMBOL zum
	 BOUND_BINDING auf nach need_new_block. */
      /* if (is_parameter(c, enclosure)) c->type = BINDING; */
      /* if (is_bound_parameter(c, enclosure)) c->type = BOUND_BINDING; */
    } else {			/* compute the enclosure anew */
      enclosure = enclosing_block(*c, blocks, *blocks_count);
    }

    /* i.e. ist das Ding jetzt der WERT für einen Bound Parameter?
       ich frag hier ob das enclosing Zeug von einem bound param
       ist ...*/
    /* enclosure ist hier der block von bound_param */
    /* is_binding(enclosure->bricks + 0, enclosure->plate) */
    if ((*enclosure->bricks)->type == BINDING || (*enclosure->bricks)->type == BOUND_BINDING) {
      (*enclosure->bricks)->type = BOUND_BINDING;
      if (enclosure->max_absorption_capacity == 1)
	enclosure->max_absorption_capacity = 0;
      else {
	enclosure = enclosure->plate;
      }
    }
    /* enclosure->plate->bricks->token.str; */
    
    /* If the computed enclosing block is a lambda-parameter and it
       has no more absorption capacity then reset the enclosing block
       to be the enclosing block of the lambda-parameter block
       i.e. the lambda block itself (imply that the current item is
       the return-expression of the lambda-block). If the parameter
       block still has absorption capacity (i.e. it's single
       default-argument) the computed enclosing block is correct, only
       decrement it's absorption capacity. */
    /* &(enclosing_block->bricks[0]) */
    
    if (need_new_block(c, enclosure) || c->type==BINDING) {
      if ((blocks = realloc(blocks, (*blocks_count + 1) * sizeof (struct Plate *))) != NULL) {

	struct Plate *newblock = malloc(sizeof *newblock);
	newblock->bricks=malloc(sizeof (struct Brick *));
	struct Environment *newenv = malloc(sizeof *newenv);
	newblock->id = blockid++;
	*newblock->bricks = c;
	newblock->size = 1;
	newblock->plate = enclosure;
	*newenv = (struct Environment){
	  .enclosing_env = enclosure->env,
	  .hash_table = g_hash_table_new(g_str_hash, g_str_equal)
	};
	newblock->env = newenv;

	/* set the new block's content */
	newblock->elements = malloc(sizeof (struct Plate_element));
	(*(newblock->elements)).type = BRICK;
	(*(newblock->elements)).cell_item = c;
	
	*(blocks + (*blocks_count)++) = newblock;
	
	/* keep an eye on this if its THE BEGINNING of a lambda */
	if (is_lambda_head(*c)) {
	  newblock->islambda = true; /* is a lambda-block */
	  newblock->arity = 0; /* default is null-arity */
	  active_binding_plate = newblock;
	  c->type=LAMBDA;
	} else {
	  newblock->islambda = false;
	}
	/* LET Block */
	if (is_association(c)) {
	  active_binding_plate = newblock;
	}

	if (is_binding(c, enclosure) || c->type==BINDING) {
	  /* printf("Christoph Seibert"); */
	  newblock->max_absorption_capacity = 1;	/* ist maximal das default argument wenn vorhanden */
	  /* enhance the type from simple SYMBOL to BOUND_BINDING */
	  /* c->type = BOUND_BINDING; */
	}
		
	/* das ist doppel gemoppelt, fass die beiden unten zusammen... */
	if ((enclosure->elements = realloc(enclosure->elements, (enclosure->size+1) * sizeof(struct Plate_element))) != NULL) {
	  (*(enclosure->elements + enclosure->size)).type = PLATE;
	  (*(enclosure->elements + enclosure->size)).block_item = newblock;
	  enclosure->size++;
	}
      } else exit(EXIT_FAILURE); /* blocks realloc failed */      
    } else {			 /* no need for a new block, just a single lonely cell */
      if ((enclosure->elements = realloc(enclosure->elements, (enclosure->size+1) * sizeof(struct Plate_element))) != NULL) {
	(*(enclosure->elements + enclosure->size)).type = BRICK;
	(*(enclosure->elements + enclosure->size)).cell_item = c;
      }
      c->plate = enclosure;
      /* enclosure->bricks[enclosure->size] = *c; */
      if ((enclosure->bricks = realloc(enclosure->bricks, (enclosure->size + 1) * sizeof (struct Brick *))) != NULL) {
	*(enclosure->bricks + enclosure->size) = c;
      }
      enclosure->size++;
    }
    c = c->next;
  }
  return blocks;
}
void print_code_ast(struct Plate *root, int depth);
char *stringify_block_item_type(enum Plate_element_type t);

/* append elt as the last item to plt's elements */
void append_element(struct Plate_element *elt, struct Plate *plt)
{  
  /* ******************************* */
  if ((plt->elements = realloc(plt->elements,
			    (plt->size+1) * sizeof (struct Plate_element))) != NULL) {
	(plt->elements + plt->size)->type = elt->type;
	if (elt->type == BRICK) {
	  (plt->elements + plt->size)->cell_item = elt->cell_item;
	  /* auch in bricks */
	  if ((plt->bricks = realloc(plt->bricks, (plt->size+1)*sizeof (struct Brick *))) != NULL)
	    *(plt->bricks + plt->size) = elt->cell_item;
	}
	else			/* hoffentlich PLATE!!! */
	  (plt->elements + plt->size)->block_item = elt->block_item;
	plt->size++;
  }
  /* ************************ */
}

/* struct Plate *remove_element(struct Plate_element *elt, struct Plate *plt) */
/* { */
/*   for (int i = 0; i < plt->size; i++) { */
    
/*   } */
/*   return elt; */
/* } */
  
/* testen wir mal die Semantik von Lambda expressions */
void amend_lambda_semantics(struct Plate *root)
{
  for (int i = 0; i < root->size; i++) {
    switch (root->elements[i].type) {
      /* a lambda can only ever be a plate */
    case BRICK: break;
    case PLATE:
      if (brick_type(*root->elements[i].block_item->bricks) == LAMBDA) {
	int sz = (*root->elements[i].block_item).size;
	printf("====%p=====\n", (void *)*root->elements[i].block_item->bricks);
	/* for (int j = 1; j < sz-1; j++) { */
	/*   /\* bestimmt plates *\/ */
	/*   printf("->%d %s\n", j,(*root->elements[i].block_item->elements[j].block_item->bricks)->token.str); */
	/* } */
	struct Plate_element *body = root->elements[i].block_item->elements + (sz - 1); /*  */
	/* body teil */
	switch (body->type) {
	case BRICK: break;	/* sieht gut aus (a cell), klassen wir mal so bleiben */
	case PLATE:
	  /* printf("??%s\n", stringify_type(brick_type(*body->block_item->bricks))); */
	  if (brick_type(*body->block_item->bricks) == BINDING) {
	    fprintf(stderr, "binding '%s' kann nicht ende von deinem lambda sein\n",
		    (*body->block_item->bricks)->token.str);
	    exit(EXIT_FAILURE);
	  } else if (brick_type(*body->block_item->bricks) == BOUND_BINDING) { /* .x ... */
	    
	    /* (root->elements[i].block_item->size)++; */
	    
	    /* block_item hat genau 2 teile:  */
	    struct Plate_element *retstat = body->block_item->elements + 1; /* ... */
	    /* struct Brick *retstat = body->block_item->bricks + 1; */
	    if (retstat->type ==BRICK) {
	      retstat->cell_item->plate = body->block_item->plate;
	      /* body->block_item->size--; */
	      body->block_item->elements->cell_item->type = BINDING;
	      printf("retstat %s %s %p %d %s\n", retstat->cell_item->token.str,
		     (*retstat->cell_item->plate->bricks)->token.str,
		     (void *)*retstat->cell_item->plate->bricks,
		     root->elements[i].block_item->size, (*root->elements[i].block_item->bricks)->token.str
		     );
	      append_element(retstat, body->block_item->plate);
	    }
	  }
	  printf("->%s\n", (*root->elements[i].block_item->elements[sz-1].block_item->bricks)->token.str);
	  break;
	}
      }
      amend_lambda_semantics(root->elements[i].block_item);
      break;
    default: break;
    }
  }
}

void free_parser_blocks(struct Plate **blocks, int blocks_count)
{
  /* the first pointer in **blocks points to the global_block, thats why we
     can't free *(blocks + 0), as global_block is created on the
     stack in main(). that first pointer will be freed after the for loop. */
  for (int i = 1; i < blocks_count; i++) {
    free((*(blocks + i))->elements);
    free(*(blocks + i));
  }
  /* free the content of the toplevel block, since it surely
     containts something when the parsed string hasn't been an empty
     string! */
  free((*blocks)->elements);
  free(blocks);
}
