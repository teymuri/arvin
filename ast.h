#ifndef LET_AST_H
#define LET_AST_H

struct Plate **parse__Hp(struct Plate *global_block, struct Brick *linked_cells_root, int *blocks_count);
void free_parser_blocks(struct Plate **blocks, int blocks_count);

#endif	/* LET_AST_H */
