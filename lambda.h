#ifndef LET_LAMBDA_H
#define LET_LAMBDA_H

#include "env.h"
/* #include "const_item.h" */

struct Lambda {
  struct Env *lambda_env;
  struct Cons_item *return_expr;	/* the return statement */
};

#endif	/* LET_LAMBDA_H */
