#ifndef LET_CORE_H
#define LET_CORE_H


#include "type.h"

struct Tila_data *pret(struct Tila_data *);
struct Tila_data *GJ(void);
/* bool is_define(struct Cons *); */
/* extern bool is_association(struct Unit *); */
/* extern bool is_bound_parameter(struct Unit *, struct Cons *); */
extern bool isbuiltin(struct Unit *);

#endif	/* LET_CORE_H */
