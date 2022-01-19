#include <glib.h>
#include <stdio.h>
#include "let_data.h"
#include "cons.h"
/* #include "const_item.h" */


#define AST_PRINTER_CONS_STR_TL "[BASECONS Brick(%s) Size(%d) ENV(SZ:%d ID:%d)%p Arity(%d)]\n"
#define AST_PRINTER_CONS_STR "[CONS Brick(%s) Size(%d) ENV(SZ:%d ID:%d)%p Arity(%d)]\n"
#define AST_PRINTER_ATOM_STR "[ATOM(%s) Type(%s)]\n"

void print_indent(int i)
{
  int n = 2;
  char s[(i*n)+1];
  for (int k =0; k<i;k++) {
    s[k*n] = '|';
    s[(k*n)+1]=' ';
    /* s[(k*n)+2]=' '; */
  }
  s[(i*n)] = '\0';
  printf("%s", s);
}

#define AST_PRINTER_UNIT_FORMAT "[tokstr(%s) type(%s) nadd(%p) uadd(%p) sz(%d) atom(%d)]"
gboolean print_node(GNode *node, gpointer data) {
  print_indent((guint)g_node_depth(node) - 1);
  printf(AST_PRINTER_UNIT_FORMAT,	 
	 ((struct Atom *)node->data)->token.str,
	 stringify_type(atom_type((struct Atom *)node->data)),
	 (gpointer)node,
	 (gpointer)node->data,
	 g_node_n_children(node),
	 ((struct Atom *)node->data)->is_atomic
	 );
  puts("");
  return false;
}
void print_ast3(GNode *root) {
  g_node_traverse(root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)print_node, NULL);
}

/* void print_code_ast(struct Cons *root, int depth) /\* This is the written code part *\/ */
/* /\* startpoint is the root block *\/ */
/* { */
/*   for (int i = 0; i < root->size; i++) { */
/*     switch (root->elts[i].type) { */
/*     case ATOM: */
/*       print_indent(depth); */
/*       printf(AST_PRINTER_ATOM_STR, */
/* 	     root->elts[i].the_unit->token.str, */
/* 	     stringify_type(atom_type(root->elts[i].the_unit)) */
/* 	     ); */
/*       break; */
/*     case CONS: */
/*       print_indent(depth); */
/*       printf(AST_PRINTER_CONS_STR, */
/* 	     root->elts[i].the_const->bricks[0]->token.str, */
/* 	     root->elts[i].the_const->size, */
/* 	     /\* root->elts[i].b->env ? root->elts[i].b->env->symcount : -1, *\/ */
/* 	     root->elts[i].the_const->env ? -1 : -1, */
/* 	     root->elts[i].the_const->env ? root->elts[i].the_const->env->id : -1, */
/* 	     root->elts[i].the_const->env ? (void *)root->elts[i].the_const->env : NULL, */
/* 	     root->elts[i].the_const->islambda ? root->elts[i].the_const->arity : -1 */
/* 	     ); */
/*       print_code_ast(root->elts[i].the_const, depth+1); */
/*       break; */
/*     default: */
/*       print_indent(depth); */
/*       printf("[Invalid Content %d] %s %s\n", i,root[i].bricks[0]->token.str, root[i].elts[0].the_unit->token.str); */
/*       break; */
/*     } */
/*   } */
/* } */

/* void print_ast(struct Cons *root) */
/* { */
/*   /\* root's (the toplevel block) elts is a the_const of */
/*      type ATOM, so when iterating over root's elts this ATOM */
/*      will be printed but there will be no CONS printed on top of that */
/*      ATOM, thats why we are cheating here and print a CONS-Like on */
/*      top of the whole ast. *\/ */
/*   printf(AST_PRINTER_CONS_STR_TL, */
/*  	 root->elts->the_unit->token.str, */
/*  	 root->size, */
/* 	 /\* root->env ? root->env->symcount : -1, *\/ */
/* 	 root->env ? -1 : -1, */
/* 	 root->env ? root->env->id : -1, */
/* 	 root->env ? (void *)root->env : NULL, */
/* 	 root->islambda ? root->arity : -1 */
/* 	 ); */
/*   print_code_ast(root, 1); */
/* } */

/* string representation of data, this is the P in REPL */
/* data arg is the evaluated expression (is gone through eval already) */
void print(struct Let_data *data)
{
  switch (data->type) {
  case INTEGER:
    printf("%d", data->value.dataslot_int);
    break;
  case FLOAT:
    printf("%f", data->value.dataslot_float);
    break;
  case LAMBDA:
    printf("lambda...");
    break;
  default:
    break;
  }
  puts("");
}
