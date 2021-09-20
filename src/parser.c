/* #include <string.h> */
/* #include "hashmap.h" */
/* #include "funcs.h" */



/* int sym_compare(const void *a, const void *b, void *udata) { */
/*     const struct symbol *ua = a; */
/*     const struct symbol *ub = b; */
/*     return strcmp(ua->name, ub->name); */
/* } */
/* uint64_t symhash(const void *item, uint64_t seed0, uint64_t seed1) { */
/*     const struct symbol *s = item; */
/*     return hashmap_sip(s->name, strlen(s->name), seed0, seed1); */
/* } */

/* struct hashmap *make_functab(void) */
/* { */
/*   struct hashmap *hm = hashmap_new(sizeof(struct symbol), 0, 0, 0, */
/* 				   symhash, sym_compare, NULL); */
/*   hashmap_set(hm, &(struct symbol){.name="int_plus", .lambda=int_plus}); */
/*   return hm; */
/* } */

/* int main() */
/* { */
/*   struct hashmap *funcs = make_functab(); */
/*   struct symbol *s; */
/*   s = hashmap_ get(funcs, &(struct symbol) {.name="int_plus", .lambda=int_plus}); */
/*   free(funcs); */
/*   return 0; */
/* } */





struct sexpr {
  char type;
  void *data;
  struct sexpr *next;
};

/* struct symbol { */
/*   char *name; */
/*   struct sexpr *(*lambda)(struct sexpr *); */
/* }; */

int main()
{
  
  return 0;
}
