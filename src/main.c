#include <stdio.h>
#include <string.h>


#define VOIDSYM ""		/* no symbol is allowed to be called this */

typedef struct {
  char *symname;
  void *symvalue;
} symbol;

char *symnames[] = {
  "pret", "name", VOIDSYM
};

size_t interened_symbols = 2;

void intsym(symbol *s)
{
  int is_interened = 0, i = 0;
  char *symname = s->symname;
  while (strcmp(symnames[i], VOIDSYM))		/* is not the end */
    if (!strcmp(symname, symnames[i])) {
      is_interened = 1;
      break;
    } else i++;
  if (is_interened)
    printf("%s is interened\n", symname);
  else {
    printf("fresh symbol %s\n", symname);
    symnames[interened_symbols] = symname;
    interened_symbols++;
    symnames[interened_symbols] = VOIDSYM;
  }
}

int main()
{
  symbol foo = {"FOO", "foo"};
  intsym(&foo);
  symbol bar = {"BAR", "bar"};
  intsym(&bar);
  return 0;
}
