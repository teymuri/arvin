#ifndef __PRINT_H
#define __PRINT_H

#include <stdio.h>
/* #include "type.h" */


void print_indent(int i);
void print_code_ast(struct block *root, int depth);
void print_ast(struct block *root);
void print(struct letdata *data);

#endif
