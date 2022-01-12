#ifndef LET_SYMBOL_H
#define LET_SYMBOL_H

#include "let_data.h"

struct symbol {
  char *symbol_name;
  struct letdata *symbol_data;
};

#endif	/* LET_SYMBOL_H */
