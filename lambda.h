#ifndef LET_LAMBDA_H
#define LET_LAMBDA_H

#include "env.h"
#include "bundle_unit.h"

struct lambda {
  struct env *lambda_env;
  struct block_item *return_expr;	/* the return statement */
};

#endif	/* LET_LAMBDA_H */
