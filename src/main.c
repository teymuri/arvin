/* #include <glib.h> */
/* #include "type.h" */
/* /\* #include "read.h" *\/ */
/* /\* #include "ast.h" *\/ */
/* #include "print.h" */
/* /\* #include "eval.h" *\/ */
#include "let.h"

/* extern struct token *tokenize_lines__Hp(char **srclns, size_t lines_count, */
/* 					size_t *all_tokens_count); */
/* extern struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count, */
/* 					 size_t all_tokens_count); */
/* extern struct cell *linked_cells__Hp(struct token tokens[], size_t count); */
/* extern struct block **parse__Hp(struct block *global_block, struct cell *linked_cells_root, int *blocks_count); */
/* extern struct letdata *global_eval(struct block *root, */
/* 			    struct env *local_env, */
/* 				   struct env *global_env); */
/* extern void free_parser_blocks(struct block **blocks, int blocks_count); */
/* extern void free_linked_cells(struct cell *c); */


int main(int argc, char **argv)
{
  /* The global environment */
  struct env global_env = {
    .id = 0,
    /* g_hash_table_new returns a GHashTable* */
    .hash_table = g_hash_table_new(g_str_hash, g_str_equal),
    .enclosing_env = NULL,
    /* .symcount = 0 */
  };

  struct token tltok = {
    .str = TLTOKSTR,
    .column_start_idx = -1,
    .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .linum = -1,
    .id = 0
  };
  struct cell global_cell = {				/* cells[0] toplevel cell */
    /* car token */
    .car = tltok,
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
  
  struct block_item *global_item = malloc(sizeof (struct block_item));
  (*global_item).type = CELL;
  (*global_item).cell_item = &global_cell;

  struct block global_block = {
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
  
  int linum=1;
  char *lines[] = {
    "pret 1"
    
    /* "let jahr:= 2022 define f1 lambda jahr", */
    /* "define f2 lambda call f1", */
    /* "pret call f1", */
    /* "pret call f2" */
  };
  
  size_t all_tokens_count = 0;
  struct token *toks = tokenize_source__Hp(argv[1], &all_tokens_count);
  /* struct token *toks = tokenize_lines__Hp(lines, linum, &all_tokens_count); */
  size_t nctok_count = 0;
  struct token *nct = remove_comments__Hp(toks, &nctok_count, all_tokens_count);
  
  /* fortfahren nur wenn vom Quelcode nach dem Entfernen von
     Kommentaren was übrig geblieben ist */
  
  if (nctok_count) {
    struct cell *c = linked_cells__Hp(nct, nctok_count);
    struct cell *base = c;
    int blocks_count = 0;
    struct block **b = parse__Hp(&global_block, c, &blocks_count);
    
    /* print_ast(&global_block); */
    /* print(global_eval(&global_block, &global_env, &global_env)); */
    global_eval(&global_block, &global_env, &global_env);

    free_parser_blocks(b, blocks_count);
    free_linked_cells(base);
    free(nct);
  }
  
  /* struct cell *c = linked_cells__Hp(nct, nctok_count); */
  /* struct cell *base = c; */
  /* int blocks_count = 0; */

  /* struct block **b = parse__Hp(&global_block, c, &blocks_count); */
  /* print_ast(&global_block); */
  /* /\* print(global_eval(&global_block, &global_env, &global_env)); *\/ */
  /* /\* global_eval(&global_block, &global_env, &global_env); *\/ */

  /* free_parser_blocks(b, blocks_count); */
  /* free_linked_cells(base); */
  /* free(nct); */

  exit(EXIT_SUCCESS);
}
