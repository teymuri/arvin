/* cell = Single, block = Multiple */


#include <glib.h>
#include "read.h"
#include "bundle_unit.h"
#include "bit.h"
#include "env.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "bundle.h"
#include "print.h"
/* extern struct Token *tokenize_lines__Hp(char **srclns, size_t lines_count, */
/* 					size_t *all_tokens_count); */
/* extern struct Token *remove_comments__Hp(struct Token *toks, size_t *nctok_count, */
/* 					 size_t all_tokens_count); */
/* extern struct Bit *linked_cells__Hp(struct Token tokens[], size_t count); */
/* extern struct Bundle **parse__Hp(struct Bundle *global_block, struct Bit *linked_cells_root, int *blocks_count); */
/* extern struct letdata *eval(struct Bundle *root, */
/* 			    struct env *local_env, */
/* 				   struct env *global_env); */
/* extern void free_parser_blocks(struct Bundle **blocks, int blocks_count); */
/* extern void free_linked_cells(struct Bit *c); */
#define GLOBAL_TOKEN_STR "GLOBAL_TOKEN_STR"


int main(int argc, char **argv)
{
  /* The global environment */
  struct Env global_env = {
    .id = 0,
    /* g_hash_table_new returns a GHashTable* */
    .hash_table = g_hash_table_new(g_str_hash, g_str_equal),
    .enclosing_env = NULL,
    /* .symcount = 0 */
  };

  struct Token global_token = {
    .str = GLOBAL_TOKEN_STR,
    .column_start_idx = -1,
    .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .linum = -1,
    .id = 0
  };
  struct Bit global_cell = {				/* cells[0] toplevel cell */
    /* car token */
    .car = global_token,
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
  
  struct Bundle_unit *global_item = malloc(sizeof (struct Bundle_unit));
  (*global_item).type = CELL;
  (*global_item).cell_item = &global_cell;

  struct Bundle global_block = {
    .id = 0,
    .cells = { global_cell },
    /* env (Toplevel Environment) */
    .env = &global_env,
    /* .env = NULL, */
    .size = 1,			/* this is the toplevel cell */
    .block_enclosing_block = NULL,
    .items = global_item,
    .islambda = false,
    .arity = -1			/* invalid arity, because this is not a lambda block! */
  };
  
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
    struct Bit *c = linked_cells__Hp(nct, nctok_count);
    struct Bit *base = c;
    int blocks_count = 0;
    struct Bundle **b = parse__Hp(&global_block, c, &blocks_count);
    
    print_ast(&global_block);
    /* print(eval(&global_block, &global_env, &global_env)); */
    eval(&global_block, &global_env, &global_env);

    free_parser_blocks(b, blocks_count);
    free_linked_cells(base);
    free(nct);
  }
  
  /* struct Bit *c = linked_cells__Hp(nct, nctok_count); */
  /* struct Bit *base = c; */
  /* int blocks_count = 0; */

  /* struct Bundle **b = parse__Hp(&global_block, c, &blocks_count); */
  /* print_ast(&global_block); */
  /* /\* print(eval(&global_block, &global_env, &global_env)); *\/ */
  /* /\* eval(&global_block, &global_env, &global_env); *\/ */

  /* free_parser_blocks(b, blocks_count); */
  /* free_linked_cells(base); */
  /* free(nct); */

  exit(EXIT_SUCCESS);
}
