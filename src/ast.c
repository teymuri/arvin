#include <stdlib.h>
/* #include "ast.h" */
#include "type.h"

extern bool isbuiltin(struct cell *);


/* is b directly or indirectly embedding c? */
bool is_enclosed_in(struct cell c, struct block b)
{
  return (c.car.column_start_idx > b.cells[0].car.column_start_idx)
    && (c.car.linum >= b.cells[0].car.linum);
}


/* all blocks in which the cell is embedded. returns a pointer which
   points to pointers to block structures, so it's return value must
   be freed (which doesn't any harm to the actual structure pointers
   it points to!) */
struct block **enclosing_blocks__Hp(struct cell c, struct block **blocks,
				    int blocks_count, int *enblocks_count)
{
  struct block **enblocks = NULL;
  for (int i = 0; i < blocks_count; i++) {
    if (is_enclosed_in(c, *(blocks[i]))) {
      if ((enblocks = realloc(enblocks, (*enblocks_count + 1) * sizeof(struct block *))) != NULL)
	*(enblocks + (*enblocks_count)++) = *(blocks + i);
      else exit(EXIT_FAILURE);
    }
  }
  return enblocks;
}

/* returns the bottom line number */
int bottom_line_number(struct block **enblocks, int enblocks_count)
{
  int ln = -1;
  for (int i = 0; i < enblocks_count; i++) {
    if ((*(enblocks + i))->cells[0].car.linum > ln)
      ln = (*(enblocks + i))->cells[0].car.linum;
  }
  return ln;
}

struct block **bottommost_blocks__Hp(struct block **enblocks, int enblocks_count, int *botmost_blocks_count)
{
  int bln = bottom_line_number(enblocks, enblocks_count);
  struct block **botmost_blocks = NULL;
  for (int i = 0; i < enblocks_count; i++) {
    if ((*(enblocks + i))->cells[0].car.linum == bln) {
      if ((botmost_blocks = realloc(botmost_blocks, (*botmost_blocks_count + 1) * sizeof(struct block *))) != NULL) {
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
struct block *rightmost_block(struct block **botmost_blocks, int botmost_blocks_count)
{
  int column_start_idx = LEAST_COL_START_IDX;			/* start index */
  struct block *rmost_block = NULL;
  for (int i = 0; i < botmost_blocks_count; i++) {    
    if ((*(botmost_blocks + i))->cells[0].car.column_start_idx > column_start_idx) {
      rmost_block = *(botmost_blocks + i);
      column_start_idx = rmost_block->cells[0].car.column_start_idx;
    }
  }
  free(botmost_blocks);
  return rmost_block;
}

/* which one of the blocks is the direct embedding block of c? */
struct block *enclosing_block(struct cell c, struct block **blocks, int blocks_count)
{  
  int enblocks_count = 0;
  struct block **enblocks = enclosing_blocks__Hp(c, blocks, blocks_count, &enblocks_count);
  int botmost_blocks_count = 0;
  struct block **botmost_blocks = bottommost_blocks__Hp(enblocks, enblocks_count, &botmost_blocks_count);
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

bool looks_like_parameter(struct cell *c)
{
  return celltype(c) == SYMBOL && str_ends_with(c->car.str, ":");
}
bool looks_like_bound_parameter(struct cell *c)
{
  return celltype(c) == SYMBOL && str_ends_with(c->car.str, ":=");
}

bool is_lambda_head(struct cell c) { return !strcmp(c.car.str, LAMBDA_KW); }
bool is_association(struct cell *c)
{
  return !strcmp(c->car.str, ASSOCIATION_KEYWORD);
}

/* now we will be sure! */
bool is_parameter(struct cell *c, struct block *enclosing_block)
{
  return looks_like_parameter(c)
    && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).car.str,
						       ASSOCIATION_KEYWORD));
}
bool is_bound_parameter(struct cell *c, struct block *enclosing_block)
{
  return looks_like_bound_parameter(c)
    && (is_lambda_head(block_head(enclosing_block)) || !strcmp((block_head(enclosing_block)).car.str,
						       ASSOCIATION_KEYWORD));
}

bool is_define(struct block *b)
{
  return !strcmp(b->cells[0].car.str, ASSIGNMENT_KEYWORD);
}

/* is the direct enclosing block the bind keyword, ie we are about to
   define a new name? */
bool is_a_binding_name(struct block *b)
{
  return !strcmp(block_head(b->block_enclosing_block).car.str, ASSIGNMENT_KEYWORD);
}


bool need_new_block(struct cell *c, struct block *enclosing_block)
{
  return isbuiltin(c)
    || !strcmp(c->car.str, ASSIGNMENT_KEYWORD)
    || is_association(c)
    /* is the symbol to be defined? */
    /* || !strcmp(block_head(enclosing_block).car.str, ASSIGNMENT_KEYWORD) */
    /* is begin of a lambda expression? */
    || is_lambda_head(*c)
    /* is a lambda parameter? */
    || is_bound_parameter(c, enclosing_block) /* hier muss enclosing_block richtig entschieden sein!!! */
    || !strcmp(c->car.str, "call")
    || !strcmp(c->car.str, "pret")
    || !strcmp(c->car.str, "gj") /* geburtsjahr!!! */
    ;
}


struct block **parse__Hp(struct block *global_block, struct cell *linked_cells_root, int *blocks_count)
{
  /* this is the blocktracker in the python prototype */
  struct block **blocks = malloc(sizeof(struct block *)); /* make room for the toplevel block */
  *(blocks + (*blocks_count)++) = global_block;
  struct cell *c = linked_cells_root;
  struct block *enblock;
  struct block *active_superior_block;
  int blockid = 1;
  while (c) {
    
    /* find out the DIRECT embedding block of the current cell */
    if ((looks_like_parameter(c) || looks_like_bound_parameter(c)) && is_enclosed_in(*c, *active_superior_block)) { /* so its a lambda parameter */
      enblock = active_superior_block;
      active_superior_block->arity++;
      /* enhance the type of the parameter symbol. */
      /* ACHTUNG: wir setzen den neuen Typ für bound param nicht hier,
	 denn is_bound_parameter fragt ab ob das Cell vom Typ SYMBOL
	 ist, was wiederum unten im need_new_block eine Rolle
	 spielt. Deshalb verschieben wir das Setzen vom Typ von SYMBOL zum
	 BOUND_PARAMETER auf nach need_new_block. */
      if (is_parameter(c, enblock)) c->type = PARAMETER;
      /* if (is_bound_parameter(c, enblock)) c->type = BOUND_PARAMETER; */
    } else {
      enblock = enclosing_block(*c, blocks, *blocks_count);
    }

    if (is_bound_parameter(enblock->cells + 0, enblock->block_enclosing_block)) {
      /* i.e. ist das Ding jetzt der WERT für einen Bound Parameter?
	 ich frag hier ob das enclosing Zeug von einem bound param
	 ist ...*/
      /* enblock ist hier der block von bound_param */
      if (enblock->max_absorption_capacity == 1) enblock->max_absorption_capacity = 0;
      else enblock = enblock->block_enclosing_block;
    }
    
    /* If the computed enclosing block is a lambda-parameter and it
       has no more absorption capacity then reset the enclosing block
       to be the enclosing block of the lambda-parameter block
       i.e. the lambda block itself (imply that the current item is
       the return-expression of the lambda-block). If the parameter
       block still has absorption capacity (i.e. it's single
       default-argument) the computed enclosing block is correct, only
       decrement it's absorption capacity. */
    /* &(enclosing_block->cells[0]) */
    
    if (need_new_block(c, enblock)) {
      if ((blocks = realloc(blocks, (*blocks_count + 1) * sizeof(struct block *))) != NULL) {

	struct block *newblock = malloc(sizeof *newblock);
	struct env *newenv = malloc(sizeof *newenv);
	newblock->id = blockid++;
	newblock->cells[0] = *c;
	newblock->size = 1;
	newblock->block_enclosing_block = enblock;
	*newenv = (struct env){
	  .enclosing_env = enblock->env,
	  .hash_table = g_hash_table_new(g_str_hash, g_str_equal)
	};
	newblock->env = newenv;

	/* set the new block's content */
	newblock->items = malloc(sizeof (struct block_item));
	(*(newblock->items)).type = CELL;
	(*(newblock->items)).cell_item = c;
	
	*(blocks + (*blocks_count)++) = newblock;
	
	/* keep an eye on this if its THE BEGINNING of a lambda */
	if (is_lambda_head(*c)) {
	  newblock->islambda = true; /* is a lambda-block */
	  newblock->arity = 0; /* default is null-arity */
	  active_superior_block = newblock;
	} else {
	  newblock->islambda = false;
	}
	/* LET Block */
	if (is_association(c)) {
	  active_superior_block = newblock;
	}

	if (is_bound_parameter(c, enblock)) {
	  newblock->max_absorption_capacity = 1;	/* ist maximal das default argument wenn vorhanden */
	  /* enhance the type from simple SYMBOL to BOUND_PARAMETER */
	  c->type = BOUND_PARAMETER;
	}
		
	/* das ist doppel gemoppelt, fass die beiden unten zusammen... */
	if ((enblock->items = realloc(enblock->items, (enblock->size+1) * sizeof(struct block_item))) != NULL) {
	  (*(enblock->items + enblock->size)).type = BLOCK;
	  (*(enblock->items + enblock->size)).block_item = newblock;
	  enblock->size++;
	}
      } else exit(EXIT_FAILURE); /* blocks realloc failed */      
    } else {			 /* no need for a new block, just a single lonely cell */
      if ((enblock->items = realloc(enblock->items, (enblock->size+1) * sizeof(struct block_item))) != NULL) {
	(*(enblock->items + enblock->size)).type = CELL;
	(*(enblock->items + enblock->size)).cell_item = c;
      }
      c->cell_enclosing_block = enblock;
      enblock->cells[enblock->size] = *c;
      enblock->size++;
    }
    c = c->cdr;
  }
  return blocks;
}

void free_parser_blocks(struct block **blocks, int blocks_count)
{
  /* the first pointer in **blocks points to the global_block, thats why we
     can't free *(blocks + 0), as global_block is created on the
     stack in main(). that first pointer will be freed after the for loop. */
  for (int i = 1; i < blocks_count; i++) {
    free((*(blocks + i))->items);
    free(*(blocks + i));
  }
  /* free the content of the toplevel block, since it surely
     containts something when the parsed string hasn't been an empty
     string! */
  free((*blocks)->items);
  free(blocks);
}
