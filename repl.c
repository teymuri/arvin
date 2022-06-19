
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>

#include "read.h"
#include "print.h"
#include "ast.h"
#include "eval.h"
#include "unit.h"
#include "token.h"

int
test_repl(struct Unit tl_unit) {
    printf("(use this repl only for test purposes!)\n");
    // Write C code here
    char *x=NULL;
    char **xs=NULL;
    int ch, ch2;
    int i=0;

    size_t lines_count = 0;
    size_t all_tokens_count =0;
    struct Token *toks;
    size_t polished_tokens_count;
    struct Token *polished_tokens;
    GList *unit_link;
    GNode *ast3;
    while (1) {
        free(xs);
        xs = NULL;
        lines_count=0;
        while ((ch=getchar()) != EOF){   /* read/print "abcde" from stdin */
            if (ch != '\n') {
                x=realloc(x,i+1);
                *(x+i)=ch;
                i++;
            } else {
                x=realloc(x,i+1);
                *(x+i)='\0';
                if (ch2 == '\n') {
                    all_tokens_count = 0;
                    toks = tokenize_lines__Hp(xs, lines_count, &all_tokens_count);
                    polished_tokens_count = 0;	/*  */
                    polished_tokens = remove_comments(toks, &polished_tokens_count, all_tokens_count);
                    if (polished_tokens_count) {
                        unit_link = unit_list(polished_tokens, polished_tokens_count);
                        unit_link = g_list_prepend(unit_link, &tl_unit);
                        ast3 = parse3(unit_link);
                        print(eval3(ast3, tl_unit.env));
                    }
                    break;                
                }
                else {
                    xs=realloc(xs, lines_count+1);
                    char *xcp=malloc(i);
                    strcpy(xcp, x);
                    *(xs+lines_count)=xcp;
                    free(x);
                    x = NULL;
                    i=0;
                    lines_count++;
                }
            }
            ch2=ch;
        }
    }    
    return 0;
}
