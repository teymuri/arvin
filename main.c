/* cell = Single, block = Multiple */

#include <stdio.h>
#include <glib.h>
#include "read.h"
/* #include "const_item.h" */
#include "atom.h"
#include "env.h"
#include "ast.h"
#include "token.h"
#include "eval.h"
#include "cons.h"
#include "print.h"
/* #include "unit_type.h" */
/* extern struct Token *tokenize_lines__Hp(char **srclns, size_t lines_count, */
/* 					size_t *all_tokens_count); */
/* extern struct Token *polish_tokens(struct Token *toks, size_t *nctok_count, */
/* 					 size_t all_tokens_count); */
/* extern struct Atom *linked_cells__Hp(struct Token tokens[], size_t count); */
/* extern struct Cons **parse__Hp(struct Cons *tl_cons, struct Atom *linked_cells_root, int *blocks_count); */
/* extern struct letdata *eval(struct Cons *root, */
/* 			    struct env *local_env, */
/* 				   struct env *toplevel_env); */
/* extern void free_parser_blocks(struct Cons **blocks, int blocks_count); */
/* extern void free_linked_cells(struct Atom *c); */
#define TOPLEVEL_TOKEN_STR "TLTOKSTR"
#define TL_ATOM_UUID 0
/* void pr(gpointer l, gpointer p) */
/* { */
/*   printf("->%s\n", l->data); */
/* } */

void F(struct Atom *link) {
  printf("->%s\n", link->token.str);
}

int main(int argc, char **argv)
{
  /* The global environment */
  struct Env toplevel_env = {
    .id = 0,
    /* g_hash_table_new returns a GHashTable* */
    .hash_table = g_hash_table_new(g_str_hash, g_str_equal),
    .enclosing_env = NULL,
    /* .symcount = 0 */
  };

  struct Token tltok = {
    .str = TOPLEVEL_TOKEN_STR,
    .col_start_idx = -1,
    .column_end_idx = 100,		/* ???????????????????????????????????????? set auf maximum*/
    .line = -1,
    .id = 0
  };
  struct Atom toplevel_atom = {				/* bricks[0] toplevel cell */
    .uuid = TL_ATOM_UUID,					/*  */
    /* token token */
    /* .unit_t = ATOM, */
    .env = &toplevel_env,
    .token = tltok,
    /* next cell pointer */
    .next = NULL,
    /* in block next */
    .in_block_cdr = NULL,
    /* type ??? */
    .type = UNDEFINED,
    .linker = NULL,			/* linker */
    .ival = 0,			/* ival */
    .fval = 0.0			/* fval */
  };
  /* kann weg!!!!!!!!!!!!!!!!!!!!!!!111 */
  /* struct Cons_item *gitem = malloc(sizeof (struct Cons_item)); */
  /* (*gitem).type = ATOM; */
  /* (*gitem).the_unit = &toplevel_atom; */
  /* Alternative mit gslist */
  /* GSList *gatom_sll = NULL; */
  /* gatom_sll = g_slist_append(gatom_sll, &toplevel_atom); */

  /* top-level construct */
  /* struct Cons tl_cons = { */
  /*   .id = 0, */
  /*   /\* .bricks = &toplevel_atom, *\/ */
  /*   /\* env (Toplevel Environment) *\/ */
  /*   .env = &toplevel_env, */
  /*   /\* .env = NULL, *\/ */
  /*   .size = 1,			/\* this is the toplevel cell *\/ */
  /*   .enclosure = NULL, */
  /*   /\* .elts = gitem,		/\\* kann weg *\\/ *\/ */
  /*   .islambda = false, */
  /*   .arity = -1		/\* invalid arity, because this is not a lambda block! *\/ */
  /* }; */
  /* tl_cons.bricks = malloc(sizeof (struct Atom *)); */
  /* *tl_cons.bricks = &toplevel_atom; */
  
  /* int line=1; */
  /* char *lines[] = { */
  /*   "pret 1" */
    
  /*   /\* "let jahr:= 2022 define f1 lambda jahr", *\/ */
  /*   /\* "define f2 lambda call f1", *\/ */
  /*   /\* "pret call f1", *\/ */
  /*   /\* "pret call f2" *\/ */
  /* }; */
  
  size_t all_tokens_count = 0;
  struct Token *toks = tokenize_source__Hp(argv[1], &all_tokens_count);
  /* struct Token *toks = tokenize_lines__Hp(lines, line, &all_tokens_count); */
  size_t nctok_count = 0;
  size_t polished_tokens_count = 0;	/*  */
  struct Token *polished_tokens = polish_tokens(toks, &polished_tokens_count, all_tokens_count);
  
  /* fortfahren nur wenn vom Quelcode nach dem Entfernen von
     Kommentaren was übrig geblieben ist */
  
  if (polished_tokens_count) {
    GSList *atoms = units_linked_list(polished_tokens, polished_tokens_count);
    /* printf("**%d %s\n", g_slist_length(atoms), */
    /* 	   ((struct Atom *)(atoms+3)->data)->token.str); */
    
    /* /\* g_slist_foreach(atoms, (GFunc)pr, NULL); *\/ */
    /* for (GSList *lst = atoms; lst; lst=lst->next) { */
    /*   printf("-%p-->%s\n", atoms, ((struct Atom *)atoms->data)->token.str); */
    /*   printf("-%p-->%s\n", lst, ((struct Atom *)lst->data)->token.str); */
    /*   puts(""); */
    /* } */

    
    /* struct Atom *c = linked_cells__Hp(polished_tokens, nctok_count); */
    /* struct Atom *base = c; */
    /* int blocks_count = 0; */
    /* struct Cons **ast = parse__Hp(&tl_cons, c, &blocks_count); */
    atoms = g_slist_prepend(atoms, &toplevel_atom);
    /* g_slist_foreach(atoms, (GFunc)F, NULL); */
    GNode *ast3 = parse3(atoms);
    print_ast3(ast3);
    
    /* amend_lambda_semantics(&tl_cons); */
    
    /* print_ast(&tl_cons); */
    /* /\* print(eval(&tl_cons, &toplevel_env, &toplevel_env)); *\/ */
    /* eval(&tl_cons, &toplevel_env, &toplevel_env); */

    /* free_parser_blocks(ast, blocks_count); */
    /* free_linked_cells(base); */
    /* free(polished_tokens); */
  }
  
  /* struct Atom *c = linked_cells__Hp(polished_tokens, nctok_count); */
  /* struct Atom *base = c; */
  /* int blocks_count = 0; */

  /* struct Cons **b = parse__Hp(&tl_cons, c, &blocks_count); */
  /* print_ast(&tl_cons); */
  /* /\* print(eval(&tl_cons, &toplevel_env, &toplevel_env)); *\/ */
  /* /\* eval(&tl_cons, &toplevel_env, &toplevel_env); *\/ */

  /* free_parser_blocks(b, blocks_count); */
  /* free_linked_cells(base); */
  /* free(polished_tokens); */

  exit(EXIT_SUCCESS);
}
