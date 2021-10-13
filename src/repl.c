

/* read */
/* eval */
/* print */


#include <stdio.h>
#include <stdlib.h>

char *read_line__Hp(int *len)
{
  int c;
  char *lineptr = NULL;
  while ((c = getchar()) != EOF && c != '\n') {
    if ((lineptr = realloc(lineptr, ++*len)) != NULL)
      lineptr[*len - 1] = c;
    else
      exit(EXIT_FAILURE);
  }
  lineptr = realloc(lineptr, *len + 1);
  lineptr[*len] = '\0';
  return lineptr;
}

void free_read_line(char *l) { free(l); }

int main()
{
  char *s;
  int len = 0;
  s = read_line__Hp(&len);
  printf("*%d:%s:\n", len, s);
  free_read_line(s);
  return 0;
}
