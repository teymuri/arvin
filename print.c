/* #include "print.h" */
#include <stdio.h>
#include "let_data.h"
#include "bundle.h"
#include "bundle_unit.h"
/* #include "let.h" */

#define AST_PRINTER_BLOCK_STR_TL "[!BLOCK HEAD(%s) SIZE(%d) ENV(SZ:%d ID:%d)%p ARITY(%d)]\n"
#define AST_PRINTER_BLOCK_STR "[BLOCK HEAD(%s) SIZE(%d) ENV(SZ:%d ID:%d)%p ARITY(%d)]\n"
#define AST_PRINTER_CELL_STR "[CELL(%s) TYPE(%s)]\n"

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

void print_code_ast(struct block *root, int depth) /* This is the written code part */
/* startpoint is the root block */
{
  for (int i = 0; i < root->size; i++) {
    switch (root->items[i].type) {
    case CELL:
      print_indent(depth);
      printf(AST_PRINTER_CELL_STR,
	     root->items[i].cell_item->car.str,
	     stringify_cell_type(celltype(root->items[i].cell_item))
	     );
      break;
    case BLOCK:
      print_indent(depth);
      printf(AST_PRINTER_BLOCK_STR,
	     root->items[i].block_item->cells[0].car.str,
	     root->items[i].block_item->size,
	     /* root->items[i].b->env ? root->items[i].b->env->symcount : -1, */
	     root->items[i].block_item->env ? -1 : -1,
	     root->items[i].block_item->env ? root->items[i].block_item->env->id : -1,
	     root->items[i].block_item->env ? (void *)root->items[i].block_item->env : NULL,
	     root->items[i].block_item->islambda ? root->items[i].block_item->arity : -1
	     );
      print_code_ast(root->items[i].block_item, depth+1);
      break;
    default:
      print_indent(depth);
      printf("[Invalid Content %d] %s %s\n", i,root[i].cells[0].car.str, root[i].items[0].cell_item->car.str);
    }
  }
}
void print_ast(struct block *root)
{
  /* root's (the toplevel block) items is a block_item of
     type CELL, so when iterating over root's items this CELL
     will be printed but there will be no BLOCK printed on top of that
     CELL, thats why we are cheating here and print a BLOCK-Like on
     top of the whole ast. */
  printf(AST_PRINTER_BLOCK_STR_TL,
 	 root->items->cell_item->car.str,
 	 root->size,
	 /* root->env ? root->env->symcount : -1, */
	 root->env ? -1 : -1,
	 root->env ? root->env->id : -1,
	 root->env ? (void *)root->env : NULL,
	 root->islambda ? root->arity : -1
	 );
  print_code_ast(root, 1);
}

/* string representation of data, this is the P in REPL */
/* data arg is the evaluated expression (is gone through eval already) */
void print(struct letdata *data)
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
