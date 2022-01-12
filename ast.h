#ifndef LET_AST_H
#define LET_AST_H

struct block **parse__Hp(struct block *global_block, struct cell *linked_cells_root, int *blocks_count);
void free_parser_blocks(struct block **blocks, int blocks_count);

#endif	/* LET_AST_H */
