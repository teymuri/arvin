#include <stdio.h>

char pret(char);
void _strcpy_(char *);

int main()
{
  char *t = "Hello";
  printf("====%p====%p\n", t, t+6);
  _strcpy_(t);
  /* printf("-> %s", t); */
  return 0;
}

char pret(char c)		/* Print and return c */
{
  printf("** %c **\n", c);
  return c;
}

void _strcpy_(char *t)
{
  printf("====%p====.\n", t);
  while (pret(*++t))
    ;
  printf("====%p====...\n", t);
}
