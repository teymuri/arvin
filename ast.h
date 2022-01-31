#ifndef LET_AST_H
#define LET_AST_H

void sanify_lambdas(GNode *);
void check_funcalls(GNode *);
void check_assocs(GNode *);
bool is_association4(struct Unit *);
GNode *parse3(GList *atoms);
bool is_pret4(struct Unit *);
bool is_lambda4(struct Unit *);
bool is_call4(struct Unit *);
bool is_association4(struct Unit *);
bool is_assignment4(struct Unit *);
bool is_funcall(struct Unit *);
#endif	/* LET_AST_H */
