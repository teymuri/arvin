#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <stdbool.h>
#include <fts.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
/* #include <ncurses.h> */
/* #include <readline/readline.h> */
/* #include <readline/history.h> */
#include "read.h"
#include "unit.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "print.h"
#include "repl.h"

#define TOPLVLTOKSTR "TLTS"     /* toplevel token string */
#define TOPLVLUID 0             /* toplevel unit id */



int
main(int argc, char **argv)
{
    size_t all_tokens_count;
    struct Token *toks;
    size_t code_tokens_count;
    struct Token *code_tokens;
    GList *unit_link;
    GNode *ast3;
    
    /* toplevel unit */
    struct Unit toplevel_unit = {
        .uuid = TOPLVLUID,
        .max_cap = -1,
        .is_atomic = false,
        .token = {
            .str = TOPLVLTOKSTR,
            .col_start_idx = -1,
            .col_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
            .line = -1,
            .id = 0
        },
        .env = g_hash_table_new(g_str_hash, g_str_equal),
        /* .env = g_hash_table_new_full(g_str_hash, */
        /*                              g_str_equal), */
        /* type ??? */
        .type = UNDEFINED,
        .ival = 0,			/* ival */
        .fval = 0.0			/* fval */
    };

    /* if (argc > 2 && !strcmp(argv[2], "-lc")) { */
    /*     /\* load core into the toplevel environment *\/ */
    /*     char *curr_dir = get_current_dir_name(); */
    /*     char *core_rel_path = "/core"; */
    /*     char core_abs_path[strlen(curr_dir) + strlen(core_rel_path) + 1]; */
    /*     strcat(core_abs_path, curr_dir); */
    /*     strcat(core_abs_path, "/core"); */
    /*     char *load_paths[] = {"./core.arv", NULL}; */
    /*     FTS *ftsp = fts_open(load_paths, FTS_LOGICAL, NULL); */
    /*     if (ftsp == NULL) { */
    /*         perror("fts_open"); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
    /*     while (1) { */
    /*         FTSENT *ent = fts_read(ftsp); // get next entry (could be file or directory). */
    /*         if (ent == NULL) { */
    /*             if (errno == 0) */
    /*                 break; // No more items, bail out of while loop */
    /*             else */
    /*             { */
    /*                 // fts_read() had an error. */
    /*                 perror("fts_read"); */
    /*                 exit(EXIT_FAILURE); */
    /*             } */
    /*         }			 */
    /*         // Given a "entry", determine if it is a file or directory */
    /*         if(ent->fts_info == FTS_D)   // We are entering into a directory */
    /*             printf("Loading %s\n", ent->fts_name); */
    /*         else if(ent->fts_info == FTS_F) { */
    /*             printf("Loading %s\n", ent->fts_path); */
    /*             char *real_path = realpath(ent->fts_path, NULL); */
    /*             /\* load core file *\/ */
    /*             all_tokens_count = 0; */
    /*             toks = tokenize_source__Hp(real_path, &all_tokens_count); */
    /*             code_tokens_count = 0;	/\*  *\/ */
    /*             code_tokens = polish_tokens(toks, &code_tokens_count, all_tokens_count); */
    /*             if (code_tokens_count) { */
    /*                 unit_link = unit_linked_list(code_tokens, code_tokens_count); */
    /*                 unit_link = g_list_prepend(unit_link, &toplevel_unit); */
    /*                 ast3 = parse3(unit_link); */
    /*                 /\* print_ast3(ast3); *\/ */
    /*                 post_parse_lambda_check(ast3); */
    /*                 post_parse_call_check(ast3); */
    /*                 post_parse_let_check(ast3); */
    /*                 /\* print_ast3(ast3); *\/ */
    /*                 eval3(ast3, toplevel_unit.env); */
    /*                 /\* print(e); *\/ */
    /*             } */
    /*             free(real_path); */
    /*         } */
    /*     } */
    /*     free(curr_dir); */
    /*     // close fts and check for error closing. */
    /*     if (fts_close(ftsp) == -1) perror("fts_close");         */
    /* } */

    /* parse options */
    int c;
    bool run_repl = false;
    while (true) {
        int opt_idx = 0;
        static struct option long_opts[] = {
            {"test-repl", no_argument, 0, 0}
        };
        c = getopt_long(argc, argv, "", long_opts, &opt_idx);
        if (c == -1) break;
        switch (c) {
        case 0:
            run_repl = true;
            break;
        }
    }
    
    if (run_repl) {
        /* test_repl(toplevel_unit); */
    } else {
        /* load the script argv[1] */

        /* source_token indicate every thing in a script/...
           including all comments. code_token on the other hand are the executing
           tokens, i.e. with comments already removed. */
        all_tokens_count = 0;
        toks = tokenize_source__Hp(argv[1], &all_tokens_count);
        code_tokens_count = 0;	/*  */
        code_tokens = polish_tokens(toks, &code_tokens_count, all_tokens_count);
        if (code_tokens_count) {
            unit_link = unit_linked_list(code_tokens, code_tokens_count);
            unit_link = g_list_prepend(unit_link, &toplevel_unit);
            ast3 = parse3(unit_link);
            print_ast3(ast3);
            eval3(ast3, toplevel_unit.env);
            /* print(eval3(ast3, toplevel_unit.env)); */
        }        
    }    
    g_hash_table_destroy(toplevel_unit.env);    
    exit(EXIT_SUCCESS);
}
