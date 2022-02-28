#ifndef TILA_AST_H
#define TILA_AST_H

void post_parse_lambda_check(GNode *);
void post_parse_call_check(GNode *);
void post_parse_let_check(GNode *);
bool is_let(struct Unit *);
GNode *parse3(GList *atoms);

bool is_lambda4(struct Unit *);
bool is_call(struct Unit *);

bool
is_call(struct Unit *);

bool is_let(struct Unit *);
bool is_define(struct Unit *);
bool is_pass(struct Unit *);
bool is_true(struct Unit *);
/* bool is_false(struct Unit *); */
/* bool is_bool(struct Unit *); */

/* c level geschwister: sind keine let funktionen: können nicht mit
   pass aufgerufen werden, haben keine keyword parameter usw... */
bool is_cpack(struct Unit *);
bool is_cith(struct Unit *);
bool is_tila_nth(struct Unit *);
bool is_tila_size(struct Unit *);
bool is_tila_show(struct Unit *);
bool is_tila_list(struct Unit *);
bool is_cond(struct Unit *);
bool is_cond_if(struct Unit *);
bool is_cond_then(struct Unit *);
bool is_cond_else(struct Unit *);
bool is_tila_add(struct Unit *);
bool is_tila_fold(struct Unit *);
#endif	/* TILA_AST_H */
