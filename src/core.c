


#include "let.h"


/* wie jede andere funktion, muss hier auch eine struct letdata pointer zurückgegeben werden */
/* das hier ist ein (interner Sprachkonstrukt) console.log ähnliches
   ding */
/* hoffentlich ist thing already evaled!!! */
struct letdata *pret(struct letdata *thing)
{
  /* puts(">"); */
  switch(thing->type) {
  case INTEGER: printf("%d", thing->value.dataslot_int); break;
  case FLOAT: printf("%f", thing->value.dataslot_float); break;
  case LAMBDA: printf("tbi:lambda (to be implemented)"); break;
  default: break;
  }
  puts("");
  /* puts("<"); */
  return thing;
}

struct letdata *GJ(void) {
  struct letdata *ld = malloc(sizeof *ld);
  ld->type = INTEGER;
  ld->value.dataslot_int = 1363;
  /* printf("mein geburtsjahr %d", ld->value.i); */
  return ld;
}
/* struct letdata *(*lambda_0)(); */
/* struct letdata *(*f1)(struct letdata *); */
/* struct letdata *(*f2)(struct letdata *, struct letdata *); */
/* struct lambda { */
/*   int arity; */
/* blkcont} */

char *__Builtins[] = {
  "+", "*", "-", "/"
};
int __Builtins_count = 4;


/* is the cell c a builtin? */
bool isbuiltin(struct cell *c)
{
  for (int i = 0; i < __Builtins_count; i++)
    if (!strcmp(cellstr(c), __Builtins[i]))
      return true;
  return false;
}

