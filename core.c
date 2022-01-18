#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "type.h"
#include "let_data.h"
#include "atom.h"



/* wie jede andere funktion, muss hier auch eine struct Let_data pointer zurückgegeben werden */
/* das hier ist ein (interner Sprachkonstrukt) console.log ähnliches
   ding */
/* hoffentlich ist thing already evaled!!! */
struct Let_data *pret(struct Let_data *thing)
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

struct Let_data *GJ(void) {
  struct Let_data *ld = malloc(sizeof *ld);
  ld->type = INTEGER;
  ld->value.dataslot_int = 1363;
  /* printf("mein geburtsjahr %d", ld->value.i); */
  return ld;
}
/* struct Let_data *(*lambda_0)(); */
/* struct Let_data *(*f1)(struct Let_data *); */
/* struct Let_data *(*f2)(struct Let_data *, struct Let_data *); */
/* struct Lambda { */
/*   int arity; */
/* blkcont} */

char *__Builtins[] = {
  "+", "*", "-", "/"
};
int __Builtins_count = 4;


/* is the cell c a builtin? */
bool isbuiltin(struct Atom *c)
{
  for (int i = 0; i < __Builtins_count; i++)
    if (!strcmp(cellstr(c), __Builtins[i]))
      return true;
  return false;
}

