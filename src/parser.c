/* #include <limits.h> */
/* #include "lexer.h" */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "hashmap.c/hashmap.h"

#define TOPLEVEL_TOKEN_STR "__TL"
#define TOPLEVEL_ID 0

int __Envid = 0;
int __Blockid = 0;


/* struct token __Toplevel_token = { */
/*   TOPLEVEL_TOKEN_STR,		/\* str *\/ */
/*   -1,				/\* sidx (provide an invalid start index) *\/ */
/*   ULONG_MAX,			/\* eidx (provide an unlikely value) *\/ */
/*   -1,				/\* linum (provide an invalid line number)*\/ */
/*   TOPLEVEL_ID,			/\*  *\/ */
/*   NULL				/\* comidx *\/ */
/* }; */

/* struct symbol { */
/*   void *symval; */
/*   char *symname; */
/* }; */

/* struct env { */
/*   int id; */
/*   struct env *parenv; */
/* }; */

/* struct block { */
/*   struct token head; */
/*   int id; */
/*   struct block *enclosing; */
/* }; */

/* struct block __Toplevel_block = { */
/*   __Toplevel_token,		/\* head token *\/ */
/*   TOPLEVEL_ID,			/\* id *\/ */
/* }; */

/* struct block *ast(struct token *tokens) */
/* { */  
/* } */

/* struct symbol { */
/*   char *name; */
/* }; */


/* uint64_t user_hash(const void *item, uint64_t seed0, uint64_t seed1) { */
/*     const struct symbol *symbol = item; */
/*     return hashmap_sip(symbol->name, strlen(symbol->name), seed0, seed1); */
/* } */
/* int user_compare(const void *a, const void *b, void *udata) { */
/*     const struct symbol *ua = a; */
/*     const struct symbol *ub = b; */
/*     return strcmp(ua->name, ub->name); */
/* } */
/* void fn(void) */
/* { */
/*   printf("Hello Fn"); */
/* } */
/* int main() */
/* { */
/*   struct hashmap *map = hashmap_new(sizeof(struct symbol), 0,0,0,user_hash, user_compare, NULL); */
/*   hashmap_set(map, &(struct symbol){.name="cons"}); */
/*   hashmap_set(map, &(struct symbol){.name="car"}); */
/*   hashmap_set(map, &(struct symbol){.name="cdr"}); */
/*   struct symbol *s = hashmap_get(map, &(struct symbol){.name="cons"}); */
/*   printf("symbol found = %s\n", s? s->name : "not found"); */
/*   hashmap_free(map); */
/*   return 0; */
/* } */
#define A(...) {__VA_ARGS__, }
int count(int arr[])
{
  int c = 0;
  for (int i = 0; arr[i] != '\0'; i++, c++)
    ;
  return c;
}
int sum(int n, ...)
{
  va_list valist;
  va_start(valist, n);
  int x = 10;
  while (x--)
    printf("->%d\n", va_arg(valist, int));
    /* n += va_arg(valist, int); */
  va_end(valist);
  return n;
}

int plus(int a, int b) { return a + b; }

int main()
{
  /* sum(1, 4, 5); */
  int a[] = A(-1, 0, 1, 2, 3, 4, 5);
  printf("%d\n", 0 == '\0');
  printf("c = %d\n", count(a));
  /* for (int i = 0; a[i]; i++) */
  /*   printf("%d\n", a[i]); */
  return 0;
}
