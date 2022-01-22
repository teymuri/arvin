/* cell = Single, block = Multiple */

#include <stdio.h>
#include <glib.h>
#include "read.h"
/* #include "const_item.h" */
#include "unit.h"
#include "env.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "print.h"

#define TOPLEVEL_TOKEN_STRING "_TLTS_"
#define TL_ATOM_UUID 0


int main(int argc, char **argv) {
  /* The global environment */
  struct Env toplevel_env = {
    .id = 0,
    /* g_hash_table_new returns a GHashTable* */
    .hash_table = g_hash_table_new(g_str_hash, g_str_equal),
    .enclosing_env = NULL,
    /* .symcount = 0 */
  };

  struct Token toplevel_token = {
    .str = TOPLEVEL_TOKEN_STRING,
    .col_start_idx = -1,
    .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .line = -1,
    .id = 0
  };
  struct Unit toplevel_unit = {				/* bricks[0] toplevel cell */
    .uuid = TL_ATOM_UUID,					/*  */
    .max_capacity = -1,
    /* token token */
    /* .unit_t = ATOM, */
    .env = &toplevel_env,
    .token = toplevel_token,
    /* type ??? */
    .type = UNDEFINED,
    .ival = 0,			/* ival */
    .fval = 0.0			/* fval */
  };
  
  size_t all_tokens_count = 0;
  struct Token *toks = tokenize_source__Hp(argv[1], &all_tokens_count);
  /* struct Token *toks = tokenize_lines__Hp(lines, line, &all_tokens_count); */
  size_t polished_tokens_count = 0;	/*  */
  struct Token *polished_tokens = polish_tokens(toks, &polished_tokens_count, all_tokens_count);
  
  /* fortfahren nur wenn vom Quelcode nach dem Entfernen von
     Kommentaren was übrig geblieben ist */
  
  if (polished_tokens_count) {
    GSList *atoms = units_linked_list(polished_tokens, polished_tokens_count);
    atoms = g_slist_prepend(atoms, &toplevel_unit);
    GNode *ast3 = parse3(atoms);
    ascertain_lambda_spellings(ast3);
    print_ast3(ast3);    
  }
  exit(EXIT_SUCCESS);
}
