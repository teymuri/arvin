#ifndef LET_CORE_H
#define LET_CORE_H


#include "let_data.h"

struct Let_data *pret(struct Let_data *);
struct Let_data *GJ(void);
/* bool is_define(struct Bundle *); */
/* extern bool is_association(struct Bit *); */
/* extern bool is_bound_parameter(struct Bit *, struct Bundle *); */
extern bool isbuiltin(struct Bit *);

#endif	/* LET_CORE_H */
