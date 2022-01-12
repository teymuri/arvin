#ifndef LET_LAMBDA_H
#define LET_LAMBDA_H

#include "env.h"
#include "bundle_unit.h"

struct Lambda {
  struct Env *lambda_env;
  struct BundleUnit *return_expr;	/* the return statement */
};

#endif	/* LET_LAMBDA_H */
