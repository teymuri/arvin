
#include "type.h"
#include "unit.h"

char *stringify_type(enum Type t) {
    switch (t) {
    case 0: return "Weiss net, waisch?!! vielleicht number??????";
    case 1: return "Integer";
    case 2: return "Float";
    case 3: return "Name";
    case 4: return "Lambda";
    case 5: return "Double";
    case 6: return "Binding";
    case 7: return "Bound_binding";
    case 8: return "Pack_binding";
    case 9: return "Bound_pack_binding";
    case 10: return "pack";
    case 11: return "Bool";
    case 12: return "List";
    default: return "undefined";
    }
}

bool is_of_type(struct Unit *u, enum Type t) {
    return unit_type(u) == t;
}

/* sets the appropriate slot of the destination data based on the slot
 * of the source data */
void set_data(struct Arv_data *dest, struct Arv_data *src) {
    switch (src->type) {
    case INT:
        dest->type = INT;
        dest->slots.tila_int = src->slots.tila_int;
        break;
    case FLOAT:
        dest->type = FLOAT;
        dest->slots.tila_float = src->slots.tila_float;
        break;
    case LAMBDA:
        dest->type = LAMBDA;
        dest->slots.tila_lambda = src->slots.tila_lambda;
        break;
    case BOOL:
        dest->type = BOOL;
        dest->slots.tila_bool = src->slots.tila_bool;
        break;
    case PACK:
        dest->type = PACK;
        dest->slots.pack = src->slots.pack;
        break;
    case LIST:
        dest->type = LIST;
        dest->slots.arv_tree = src->slots.arv_tree;
        break;
    default: break;
    }
}

