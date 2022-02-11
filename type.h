#ifndef TILA_TYPE_H
#define TILA_TYPE_H

#include <glib.h>
#include <stdbool.h>

enum Type {
    NUMBER = 0, INTEGER = 1, FLOAT = 2,
    NAME = 3, LAMBDA = 4,
    DOUBLE = 5,
    BINDING=6,
    BOUND_BINDING=7,	/* parameter with default argument */
    PACK_BINDING = 8,
    BOUND_PACK_BINDING = 9,
    PACK = 10,
    BOOL = 11,
    UNDEFINED
};

/* data structures of the landuage */

struct Lambda {
    int arity;
    GList *param_list;
    /* lambda environment is used at definition time  */
    GHashTable *env;
    GNode *node;			/* the lambda code */
};

struct Tila_data {
    enum Type type;
    union {
        int int_slot;
        float float_slot;
        struct Lambda *slot_lambda;
        GList *pack;
        bool slot_bool;
    } data;
};

char *stringify_type(enum Type);
void set_data_slot(struct Tila_data *, struct Tila_data *);


#endif	/* TILA_TYPE_H */
