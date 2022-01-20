#ifndef LET_AST_H
#define LET_AST_H

void ascertain_lambda_spellings(GNode *root);
GNode *parse3(GSList *atoms);
struct Cons **parse__Hp(struct Cons *global_block, struct Atom *linked_cells_root, int *blocks_count);
void amend_lambda_semantics(struct Cons *);
void free_parser_blocks(struct Cons **blocks, int blocks_count);

#endif	/* LET_AST_H */
