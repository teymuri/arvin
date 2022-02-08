#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "type.h"
#include "token.h"
#include "unit.h"



/* wie jede andere funktion, muss hier auch eine struct Let_data pointer zurückgegeben werden */
/* das hier ist ein (interner Sprachkonstrukt) console.log ähnliches
   ding */
/* hoffentlich ist thing already evaled!!! */
struct Let_data *pret(struct Let_data *thing) {
    /* puts(">"); */
    switch(thing->type) {
    case INTEGER: printf("%d", thing->data.int_slot); break;
    case FLOAT: printf("%f", thing->data.float_slot); break;
    case BOOL:printf("%s", thing->data.slot_bool ? TRUE_KW:FALSE_KW); break;
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
    ld->data.int_slot = 1363;
    /* printf("mein geburtsjahr %d", ld->data.i); */
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


