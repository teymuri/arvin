#include "lexer.h"

int __Envid = 0;
int __Blockid = 0;

struct env {
  struct env *parenv;
};

struct block {
  struct token head;
  int id;
};
