#ifndef LET_SYMBOL_H
#define LET_SYMBOL_H

#include "let_data.h"

struct Symbol {
  char *symbol_name;
  struct Let_data *symbol_data;
};

#endif	/* LET_SYMBOL_H */
