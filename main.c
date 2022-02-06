

#include <stdio.h>
#include <glib.h>
#include <stdbool.h>
#include "read.h"
#include "unit.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "print.h"

#define TOPLEVEL_TOKEN_STRING "_TLTS"
#define TOPLEVEL_UNIT_UUID 0


int main(int argc, char **argv) {
    struct Token toplevel_token = {
        .str = TOPLEVEL_TOKEN_STRING,
        .col_start_idx = -1,
        .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
        .line = -1,
        .id = 0
    };
    struct Unit toplevel_unit = {				/* bricks[0] toplevel cell */
        .uuid = TOPLEVEL_UNIT_UUID,					/*  */
        .max_capa = -1,
        .arity = -1,
        .is_atomic = false,
        .token = toplevel_token,
        .env = g_hash_table_new(g_str_hash, g_str_equal),
        /* type ??? */
        .type = UNDEFINED,
        .ival = 0,			/* ival */
        .fval = 0.0			/* fval */
    };
  
    size_t all_tokens_count = 0;
    struct Token *toks = tokenize_source__Hp(argv[1], &all_tokens_count);
    size_t polished_tokens_count = 0;	/*  */
    struct Token *polished_tokens = polish_tokens(toks, &polished_tokens_count, all_tokens_count);
  
    /* fortfahren nur wenn vom Quelcode nach dem Entfernen von
       Kommentaren was übrig geblieben ist */
  
    if (polished_tokens_count) {
        GList *unit_link = unit_linked_list(polished_tokens, polished_tokens_count);
        unit_link = g_list_prepend(unit_link, &toplevel_unit);
        GNode *ast3 = parse3(unit_link);
        print_ast3(ast3);
        sanify_lambdas(ast3);
        check_funcalls(ast3);
        post_parse_let_sanfiy(ast3);
        print_ast3(ast3);
        struct Let_data *e = eval3(ast3, toplevel_unit.env);
        /* print(e); */
    }
    exit(EXIT_SUCCESS);
}
