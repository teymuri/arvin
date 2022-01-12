#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "type.h"
#include "letdata.h"
#include "bit.h"



/* wie jede andere funktion, muss hier auch eine struct LetData pointer zurückgegeben werden */
/* das hier ist ein (interner Sprachkonstrukt) console.log ähnliches
   ding */
/* hoffentlich ist thing already evaled!!! */
struct LetData *pret(struct LetData *thing)
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

struct LetData *GJ(void) {
  struct LetData *ld = malloc(sizeof *ld);
  ld->type = INTEGER;
  ld->value.dataslot_int = 1363;
  /* printf("mein geburtsjahr %d", ld->value.i); */
  return ld;
}
/* struct LetData *(*lambda_0)(); */
/* struct LetData *(*f1)(struct LetData *); */
/* struct LetData *(*f2)(struct LetData *, struct LetData *); */
/* struct Lambda { */
/*   int arity; */
/* blkcont} */

char *__Builtins[] = {
  "+", "*", "-", "/"
};
int __Builtins_count = 4;


/* is the cell c a builtin? */
bool isbuiltin(struct Bit *c)
{
  for (int i = 0; i < __Builtins_count; i++)
    if (!strcmp(cellstr(c), __Builtins[i]))
      return true;
  return false;
}

