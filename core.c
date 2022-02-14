#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "type.h"
#include "token.h"
#include "unit.h"



/* wie jede andere funktion, muss hier auch eine struct Tila_data pointer zurückgegeben werden */
/* das hier ist ein (interner Sprachkonstrukt) console.log ähnliches
   ding */
/* hoffentlich ist thing already evaled!!! */
struct Tila_data *pret(struct Tila_data *thing) {
    /* puts(">"); */
    switch(thing->type) {
    case INTEGER: printf("%d", thing->slots.tila_int); break;
    case FLOAT: printf("%f", thing->slots.tila_float); break;
    case BOOL:printf("%s", thing->slots.tila_bool ? TRUE_KW:FALSE_KW); break;
    case LAMBDA: printf("tbi:lambda (to be implemented)"); break;
    default: break;
    }
    puts("");
    /* puts("<"); */
    return thing;
}

struct Tila_data *GJ(void) {
    struct Tila_data *ld = malloc(sizeof *ld);
    ld->type = INTEGER;
    ld->slots.tila_int = 1363;
    /* printf("mein geburtsjahr %d", ld->data.i); */
    return ld;
}
/* struct Tila_data *(*lambda_0)(); */
/* struct Tila_data *(*f1)(struct Tila_data *); */
/* struct Tila_data *(*f2)(struct Tila_data *, struct Tila_data *); */
/* struct Lambda { */
/*   int arity; */
/* blkcont} */

char *__Builtins[] = {
    "+", "*", "-", "/"
};
int __Builtins_count = 4;


