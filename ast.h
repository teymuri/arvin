#ifndef ABID_AST_H
#define ABID_AST_H

void post_parse_lambda_check(GNode *);
void post_parse_call_check(GNode *);
void post_parse_let_check(GNode *);
bool is_let(struct Unit *);
GNode *parse3(GList *atoms);

bool is_lambda4(struct Unit *);
bool is_lambda(struct Unit *);
/* bool maybe_param_with_dflt_arg(struct Unit *); */
bool is_call(struct Unit *);

bool is_call(struct Unit *);
bool is_call2(struct Unit *);

bool is_let(struct Unit *);
bool is_let2(struct Unit *);
bool is_define(struct Unit *);
bool is_pass(struct Unit *);
bool is_true(struct Unit *);
/* bool is_false(struct Unit *); */
/* bool is_bool(struct Unit *); */

/* c level geschwister: sind keine let funktionen: können nicht mit
   pass aufgerufen werden, haben keine keyword parameter usw... */
bool is_cpack(struct Unit *);
bool is_cith(struct Unit *);
bool is_nth_op(struct Unit *);
bool is_size_op(struct Unit *);
bool is_show_op(struct Unit *);
bool is_list_op(struct Unit *);
bool is_list_op2(struct Unit *);
bool is_cond(struct Unit *);
bool is_cond_if(struct Unit *);
bool is_cond_then(struct Unit *);
bool is_cond_else(struct Unit *);
bool is_add_op(struct Unit *);
bool is_add_op2(struct Unit *);
bool is_mul_op2(struct Unit *);
bool is_sub_op2(struct Unit *);
bool is_div_op2(struct Unit *);
bool is_sub_op(struct Unit *);
bool is_mul_op(struct Unit *);
bool is_div_op(struct Unit *);
bool is_exp_op(struct Unit *);
bool is_inc_op(struct Unit *);
bool is_dec_op(struct Unit *);
bool is_lfold_op(struct Unit *);



#endif	/* ABID_AST_H */
