/* #include "print.h" */
#include <stdio.h>
#include "let_data.h"
#include "plate.h"
#include "plate_element.h"


#define AST_PRINTER_PLATE_STR_TL "[BASEPLATE Brick(%s) Size(%d) ENV(SZ:%d ID:%d)%p Arity(%d)]\n"
#define AST_PRINTER_PLATE_STR "[PLATE Brick(%s) Size(%d) ENV(SZ:%d ID:%d)%p Arity(%d)]\n"
#define AST_PRINTER_BRICK_STR "[BRICK(%s) Type(%s)]\n"

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

void print_code_ast(struct Plate *root, int depth) /* This is the written code part */
/* startpoint is the root block */
{
  for (int i = 0; i < root->size; i++) {
    switch (root->elts[i].type) {
    case BRICK:
      print_indent(depth);
      printf(AST_PRINTER_BRICK_STR,
	     root->elts[i].cell_item->token.str,
	     stringify_type(brick_type(root->elts[i].cell_item))
	     );
      break;
    case PLATE:
      print_indent(depth);
      printf(AST_PRINTER_PLATE_STR,
	     root->elts[i].block_item->bricks[0]->token.str,
	     root->elts[i].block_item->size,
	     /* root->elts[i].b->env ? root->elts[i].b->env->symcount : -1, */
	     root->elts[i].block_item->env ? -1 : -1,
	     root->elts[i].block_item->env ? root->elts[i].block_item->env->id : -1,
	     root->elts[i].block_item->env ? (void *)root->elts[i].block_item->env : NULL,
	     root->elts[i].block_item->islambda ? root->elts[i].block_item->arity : -1
	     );
      print_code_ast(root->elts[i].block_item, depth+1);
      break;
    default:
      print_indent(depth);
      printf("[Invalid Content %d] %s %s\n", i,root[i].bricks[0]->token.str, root[i].elts[0].cell_item->token.str);
      break;
    }
  }
}

void print_ast(struct Plate *root)
{
  /* root's (the toplevel block) elts is a block_item of
     type BRICK, so when iterating over root's elts this BRICK
     will be printed but there will be no PLATE printed on top of that
     BRICK, thats why we are cheating here and print a PLATE-Like on
     top of the whole ast. */
  printf(AST_PRINTER_PLATE_STR_TL,
 	 root->elts->cell_item->token.str,
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
