#ifndef LET_AST_H
#define LET_AST_H

void sanify_lambdas(GNode *);
void check_funcalls(GNode *);
void post_parse_let_check(GNode *);
bool is_let(struct Unit *);
GNode *parse3(GList *atoms);
bool is_pret4(struct Unit *);
bool is_lambda4(struct Unit *);
bool is_call4(struct Unit *);
bool is_let(struct Unit *);
bool is_assignment4(struct Unit *);
bool is_funcall(struct Unit *);

/* c level geschwister: sind keine let funktionen: können nicht mit
   pass aufgerufen werden, haben keine keyword parameter usw... */
bool is_cpack(struct Unit *);
bool is_cith(struct Unit *);

#endif	/* LET_AST_H */
