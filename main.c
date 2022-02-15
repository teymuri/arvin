

#include <stdio.h>
#include <glib.h>
#include <stdbool.h>
#include "read.h"
#include "unit.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "print.h"

#define GLOBAL_TOKEN_STRING "GTS"
#define GLOBAL_UNIT_UUID 0


int main(int argc, char **argv)
{
    /* struct Token toplevel_token = { */
    /*     .str = GLOBAL_TOKEN_STRING, */
    /*     .col_start_idx = -1, */
    /*     .column_end_idx = 100,		/\* ???????????????????????????????????????? set auf maximum*\/ */
    /*     .line = -1, */
    /*     .id = 0 */
    /* }; */
    
    /* global unit */
    struct Unit gunit = {
        .uuid = GLOBAL_UNIT_UUID,
        .max_capa = -1,
        .arity = -1,
        .is_atomic = false,
        .token = {
            .str = GLOBAL_TOKEN_STRING,
            .col_start_idx = -1,
            .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
            .line = -1,
            .id = 0
        },
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
        unit_link = g_list_prepend(unit_link, &gunit);
        GNode *ast3 = parse3(unit_link);
        print_ast3(ast3);
        post_parse_lambda_check(ast3);
        post_parse_pass_check(ast3);
        post_parse_let_check(ast3);
        print_ast3(ast3);
        eval3(ast3, gunit.env);
        /* print(e); */
    }
    exit(EXIT_SUCCESS);
}
