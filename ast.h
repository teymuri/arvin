#ifndef LET_AST_H
#define LET_AST_H

void post_parse_lambda_check(GNode *);
void post_parse_pass_check(GNode *);
void post_parse_let_check(GNode *);
bool is_let(struct Unit *);
GNode *parse3(GList *atoms);
bool is_pret4(struct Unit *);
bool is_lambda4(struct Unit *);
bool is_call(struct Unit *);
bool is_let(struct Unit *);
bool is_assignment4(struct Unit *);
bool is_pass(struct Unit *);
bool is_true(struct Unit *);
/* bool is_false(struct Unit *); */
/* bool is_bool(struct Unit *); */

/* c level geschwister: sind keine let funktionen: können nicht mit
   pass aufgerufen werden, haben keine keyword parameter usw... */
bool is_cpack(struct Unit *);
bool is_tila_list(struct Unit *);
bool is_cith(struct Unit *);
bool is_tila_nth(struct Unit *);

#endif	/* LET_AST_H */
