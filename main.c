/* cell = Single, block = Multiple */


#include <glib.h>
#include "read.h"
#include "plate_element.h"
#include "brick.h"
#include "env.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "plate.h"
#include "print.h"
/* extern struct Token *tokenize_lines__Hp(char **srclns, size_t lines_count, */
/* 					size_t *all_tokens_count); */
/* extern struct Token *remove_comments__Hp(struct Token *toks, size_t *nctok_count, */
/* 					 size_t all_tokens_count); */
/* extern struct Brick *linked_cells__Hp(struct Token tokens[], size_t count); */
/* extern struct Plate **parse__Hp(struct Plate *base_plate, struct Brick *linked_cells_root, int *blocks_count); */
/* extern struct letdata *eval(struct Plate *root, */
/* 			    struct env *local_env, */
/* 				   struct env *base_plate_env); */
/* extern void free_parser_blocks(struct Plate **blocks, int blocks_count); */
/* extern void free_linked_cells(struct Brick *c); */
#define GLOBAL_TOKEN_STR "GLOBAL_TOKEN_STR"


int main(int argc, char **argv)
{
  /* The global environment */
  struct Env base_plate_env = {
    .id = 0,
    /* g_hash_table_new returns a GHashTable* */
    .hash_table = g_hash_table_new(g_str_hash, g_str_equal),
    .enclosing_env = NULL,
    /* .symcount = 0 */
  };

  struct Token base_plate_token = {
    .str = GLOBAL_TOKEN_STR,
    .column_start_idx = -1,
    .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .linum = -1,
    .id = 0
  };
  struct Brick base_plate_brick = {				/* cells[0] toplevel cell */
    /* car token */
    .car = base_plate_token,
    /* cdr cell pointer */
    .cdr = NULL,
    /* in block cdr */
    .in_block_cdr = NULL,
    /* type ??? */
    .type = UNDEFINED,
    .linker = NULL,			/* linker */
    .ival = 0,			/* ival */
    .fval = 0.0			/* fval */
  };
  
  struct Plate_element *base_plate_element = malloc(sizeof (struct Plate_element));
  (*base_plate_element).type = BRICK;
  (*base_plate_element).cell_item = &base_plate_brick;

  struct Plate base_plate = {
    .id = 0,
    /* .cells = &base_plate_brick, */
    /* env (Toplevel Environment) */
    .env = &base_plate_env,
    /* .env = NULL, */
    .size = 1,			/* this is the toplevel cell */
    .block_enclosing_block = NULL,
    .items = base_plate_element,
    .islambda = false,
    .arity = -1			/* invalid arity, because this is not a lambda block! */
  };
  base_plate.cells = malloc(sizeof (struct Brick *));
  *base_plate.cells = &base_plate_brick;
  
  /* int linum=1; */
  /* char *lines[] = { */
  /*   "pret 1" */
    
  /*   /\* "let jahr:= 2022 define f1 lambda jahr", *\/ */
  /*   /\* "define f2 lambda call f1", *\/ */
  /*   /\* "pret call f1", *\/ */
  /*   /\* "pret call f2" *\/ */
  /* }; */
  
  size_t all_tokens_count = 0;
  struct Token *toks = tokenize_source__Hp(argv[1], &all_tokens_count);
  /* struct Token *toks = tokenize_lines__Hp(lines, linum, &all_tokens_count); */
  size_t nctok_count = 0;
  struct Token *nct = remove_comments__Hp(toks, &nctok_count, all_tokens_count);
  
  /* fortfahren nur wenn vom Quelcode nach dem Entfernen von
     Kommentaren was übrig geblieben ist */
  
  if (nctok_count) {
    struct Brick *c = linked_cells__Hp(nct, nctok_count);
    struct Brick *base = c;
    int blocks_count = 0;
    struct Plate **b = parse__Hp(&base_plate, c, &blocks_count);
    
    print_ast(&base_plate);
    /* print(eval(&base_plate, &base_plate_env, &base_plate_env)); */
    eval(&base_plate, &base_plate_env, &base_plate_env);

    free_parser_blocks(b, blocks_count);
    free_linked_cells(base);
    free(nct);
  }
  
  /* struct Brick *c = linked_cells__Hp(nct, nctok_count); */
  /* struct Brick *base = c; */
  /* int blocks_count = 0; */

  /* struct Plate **b = parse__Hp(&base_plate, c, &blocks_count); */
  /* print_ast(&base_plate); */
  /* /\* print(eval(&base_plate, &base_plate_env, &base_plate_env)); *\/ */
  /* /\* eval(&base_plate, &base_plate_env, &base_plate_env); *\/ */

  /* free_parser_blocks(b, blocks_count); */
  /* free_linked_cells(base); */
  /* free(nct); */

  exit(EXIT_SUCCESS);
}
