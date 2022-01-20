#ifndef LET_PRINT_H
#define LET_PRINT_H


void print(struct Let_data *);
void print_ast3(GNode *);
gboolean print_node(GNode *node, gpointer data);

#endif	/* LET_PRINT_H */
