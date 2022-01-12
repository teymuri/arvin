#ifndef LET_AST_H
#define LET_AST_H

struct Bundle **parse__Hp(struct Bundle *global_block, struct Bit *linked_cells_root, int *blocks_count);
void free_parser_blocks(struct Bundle **blocks, int blocks_count);

#endif	/* LET_AST_H */
