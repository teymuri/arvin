#ifndef ARV_TYPE_H
#define ARV_TYPE_H

#include <glib.h>
#include <stdbool.h>

enum Type {
    NUMBER = 0, INT = 1, FLOAT = 2,
    NAME = 3, LAMBDA = 4,
    DOUBLE = 5,
    BINDING=6,
    BOUND_BINDING=7,	/* parameter with default argument */
    PACK_BINDING = 8,
    BOUND_PACK_BINDING = 9,
    PACK = 10,
    BOOL = 11,
    LIST = 12,
    UNDEFINED
};

/* data structures of the landuage */

struct Lambda {
    GList *param_list;
    /* lambda environment is used at definition time  */
    GHashTable *env;
    GNode *node;			/* the lambda code */
};

struct List {
    GList *item;
    int size;
};


struct Arv_data {
    enum Type type;
    union {
        int tila_int;
        float tila_float;
        struct Lambda *tila_lambda;
        GList *pack;
        struct List *arv_list;
        bool tila_bool;
    } slots;
};

char *stringify_type(enum Type);
void set_data(struct Arv_data *, struct Arv_data *);


#endif	/* ARV_TYPE_H */
