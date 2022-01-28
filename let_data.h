#ifndef LET_DATA_H
#define LET_DATA_H

#include <glib.h>
#include "type.h"

/* data structures of the landuage */

struct Lambda {
  GNode *expr;
  GHashTable *env;
};

struct Let_data {
  enum Type type;
  union {
    int int_slot;
    float float_slot;
    GNode *lambda_slot;
  } data;
};

#endif	/* LET_DATA_H */


/* LDAP */
/* FastAPI */
/* pydantic */
/* Docker */
/* Debian-Paketierung */

/* https://docs.software-univention.de/administrators_4.4.html.en */
/* https://docs.software-univention.de/ */

/* schwiegert@univention.de */

/* https://workadventu.re/ */
