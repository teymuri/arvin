#include <string.h>
#include <stdbool.h>

/* primitive of the lang */
static char *__Prims[] = {
  "+", "-", "*", "/",
  "lambda"
};

static int __Prims_count = 5;

bool isprim(char *s)
{
  for (int i = 0; i < __Prims_count; i++)
    if (!strcmp(s, __Prims[i]))
      return true;
  return false;
}
