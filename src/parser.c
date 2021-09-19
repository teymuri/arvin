#include <string.h>
#include "hashmap.h"
#include "funcs.h"


struct sym {
  char *name;
  void *lambda;
};

int sym_compare(const void *a, const void *b, void *udata) {
    const struct sym *ua = a;
    const struct sym *ub = b;
    return strcmp(ua->name, ub->name);
}
uint64_t symhash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct sym *s = item;
    return hashmap_sip(s->name, strlen(s->name), seed0, seed1);
}

struct hashmap *make_functab(void)
{
  struct hashmap *hm = hashmap_new(sizeof(struct sym), 0, 0, 0,
				   symhash, sym_compare, NULL);
  hashmap_set(hm, &(struct sym){.name="int_plus", .lambda=int_plus});
  return hm;
}

int main()
{
  struct hashmap *funcs = make_functab();
  struct sym *s;
  s = hashmap_get(funcs, &(struct sym) {.name="int_plus", .lambda=int_plus});
  return 0;
}
