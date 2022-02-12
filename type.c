
#include "type.h"
#include "unit.h"

char *stringify_type(enum Type t) {
    switch (t) {
    case 0: return "Weiss net, waisch?!! vielleicht number??????";
    case 1: return "integer";
    case 2: return "float";
    case 3: return "name";
    case 4: return "lambda";
    case 5: return "double";
    case 6: return "binding";
    case 7: return "bound-binding";
    case 8: return "pack-binding";
    case 9: return "bound-pack-binding";
    case 10: return "pack";
    case 11: return "bool";
    case 12: return "List";
    default: return "undefined";
    }
}

bool is_of_type(struct Unit *u, enum Type t) {
    return unit_type(u) == t;
}

/* sets the appropriate slot of the destination data based on the slot
 * of the source data */
void set_data_slot(struct Tila_data *dest, struct Tila_data *src) {
    switch (src->type) {
    case INTEGER: dest->data.int_slot = src->data.int_slot; break;
    case FLOAT: dest->data.float_slot = src->data.float_slot; break;
    case LAMBDA: dest->data.slot_lambda = src->data.slot_lambda; break;
    case BOOL: dest->data.slot_bool = src->data.slot_bool; break;
    case PACK: dest->data.pack = src->data.pack; break;
    case LIST: dest->data.list = src->data.list; break;
    default: break;
    }
}

