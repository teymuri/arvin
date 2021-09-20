#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum Cell_types { ICELL, FCELL };

struct cell {
  int car;
  struct cell *cdr;
};

struct cell *list(int n)
{
  struct cell *c = malloc(sizeof(struct cell));
  if (n > 1) {
    c->car = 3;
    c->cdr = list(n-1);
  } else {
    c->car = 5;
    c->cdr = NULL;
  }
  return c;
}
int main()
{
  /* char *args[] = {"12"}; */
  int n = 4;
  struct cell *c = list(n);
  for (int i = 0; i < n; i++){
    printf("car %d\n", c->car);
    c=c->cdr;
  }
  /* free_list(c, 1); */
  return 0;
}
