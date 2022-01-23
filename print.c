#include <glib.h>
#include <stdio.h>
#include "let_data.h"
#include "unit.h"

void print_indent(int i) {
  int n = 2;
  char s[(i*n)+1];
  for (int k =0; k<i;k++) {
    s[k*n] = '|';
    s[(k*n)+1]=' ';
  }
  s[(i*n)] = '\0';
  printf("%s", s);
}

#define UNIT_FORMAT "[tokstr(%s) type(%s) nadd(%p) uadd(%p) sz(%d) atom(%d) arity(%d) maxcap(%d)]"
gboolean print_node(GNode *node, gpointer data) {
  print_indent((guint)g_node_depth(node) - 1);
  printf(UNIT_FORMAT,	 
	 ((struct Unit *)node->data)->token.str,
	 stringify_type(unit_type((struct Unit *)node->data)),
	 (gpointer)node,
	 (gpointer)node->data,
	 g_node_n_children(node),
	 ((struct Unit *)node->data)->is_atomic,
	 ((struct Unit *)node->data)->arity,
	 ((struct Unit *)node->data)->max_capacity
	 );
  puts("");
  return false;
}
void print_ast3(GNode *root) {
  g_node_traverse(root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, (GNodeTraverseFunc)print_node, NULL);
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
