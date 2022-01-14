#ifndef LET_CORE_H
#define LET_CORE_H


#include "let_data.h"

struct Let_data *pret(struct Let_data *);
struct Let_data *GJ(void);
/* bool is_define(struct Plate *); */
/* extern bool is_association(struct Brick *); */
/* extern bool is_bound_parameter(struct Brick *, struct Plate *); */
extern bool isbuiltin(struct Brick *);

#endif	/* LET_CORE_H */
