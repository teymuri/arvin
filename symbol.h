#ifndef LET_SYMBOL_H
#define LET_SYMBOL_H

#include "let_data.h"

struct Symbol {
  char *symbol_name;
  struct LetData *symbol_data;
};

#endif	/* LET_SYMBOL_H */
