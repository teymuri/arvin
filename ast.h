#ifndef LET_AST_H
#define LET_AST_H

void sanify_lambdas(GNode *root);
GNode *parse3(GSList *atoms);

#endif	/* LET_AST_H */
