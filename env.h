#ifndef LET_ENV_H
#define LET_ENV_H

#include <glib.h>


struct Environment {
  int id;
  GHashTable *hash_table;		/* hashtable keeping known symbols */
  /* int symcount;			/\* number of symbols *\/ */
  struct Environment *enclosing_env;		/* parent environment */
};


#endif	/* LET_ENV_H */
