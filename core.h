#ifndef LET_CORE_H
#define LET_CORE_H


#include "let_data.h"

struct letdata *pret(struct letdata *);
struct letdata *GJ(void);
/* bool is_define(struct block *); */
/* extern bool is_association(struct cell *); */
/* extern bool is_bound_parameter(struct cell *, struct block *); */
extern bool isbuiltin(struct cell *);

#endif	/* LET_CORE_H */
