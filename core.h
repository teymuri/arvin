#ifndef LET_CORE_H
#define LET_CORE_H


#include "let_data.h"

struct Let_data *pret(struct Let_data *);
struct Let_data *GJ(void);
/* bool is_define(struct Cons *); */
/* extern bool is_association(struct Atom *); */
/* extern bool is_bound_parameter(struct Atom *, struct Cons *); */
extern bool isbuiltin(struct Atom *);

#endif	/* LET_CORE_H */
