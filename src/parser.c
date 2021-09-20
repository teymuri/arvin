#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct cons {
  char type;
  int icar;
  double dcar;
  void *car;
  struct cons *cdr;
};
struct cons *f(struct cons *i)
{

  struct cons *c = malloc(sizeof(struct cons));
  c->icar = 0;
  c->cdr = NULL;
  while (1) {
    c->icar += i->icar;
    if ((i->cdr == NULL)) break;
    else i = i->cdr;
  }
  return c;
}
struct symbol {
  char *name;
  struct cons *(*lambda)(struct cons *);
};
int main()
{   
  //struct cons *(*p)(struct cons *a);
  struct symbol s = {.lambda = f, .name="addition"};
  struct cons z = {.icar=3, .cdr=NULL};
  struct cons b ={.icar =10, .cdr=&z};
  struct cons a = {.icar =4, .cdr =&b};
  struct cons *ant =s.lambda(&b); /* lambda member funktioniert! */
  printf("%s %d\n", s.name, ant->icar);

  free(ant);
  return 0;
}
