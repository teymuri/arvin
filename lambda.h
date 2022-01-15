#ifndef LET_LAMBDA_H
#define LET_LAMBDA_H

#include "env.h"
#include "plate_element.h"

struct Lambda {
  struct Environment *lambda_env;
  struct Plate_element *return_expr;	/* the return statement */
};

#endif	/* LET_LAMBDA_H */
