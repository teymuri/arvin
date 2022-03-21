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
    /*     uniadic
    the property "variadic" applies to relators and operators which
    accept a variable number of arguments. however, many relators and
    operators are not variadic. they require a unique number of
    arguments. so there should exist an antonym to "variadic". my
    favorite one is "uniadic". it expresses that the arity of the
    relator or operator is uniquely determined. the term applies to
    relators and operators which accept only a definite number of
    arguments.  for instance, the logarithm is a uniadic - actually
    unary - function, while addition can be considered variadic.
    Submitted by rinat on June 29, 2016
    (https://www.synonyms.com/antonyms/VARIADIC) */
    UNIADIC_LAMBDA = 13,
    VARIADIC_LAMBDA = 14,
    
    MAND_PARAM = 15,               /* param (binding) */
    OPT_PARAM = 16,             /* param with default arg (bound binding) */
    REST_MAND_PARAM = 17,               /* rest param (pack binding) */
    REST_OPT_PARAM = 18,            /* rest param with default arg (bound pack binding) */
    CALL_OPT_REST_PARAM=19,
    
    UNDEFINED
};

/* data structures of the landuage */

struct Lambda {
    enum Type artyp;            /* arity type: fixed or indefinite */
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
