#include <stdio.h>
#include <stdlib.h>

void read_lines(char *path)
{
  FILE *stream;
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  char *lnptr = NULL;
  size_t n = 0;
  /* size_t linum = 0; */
  /* (void)null_srclns(); */
  while ((getline(&lnptr, &n, stream) != -1)) {
    /* throw away empty lines */
    if (!isempty(lnptr)) {
      /* assert(("line count too large", G_srclns_count < MAXSRCLNS)); */
      G_srclns[G_srclns_count++] = lnptr;
    }
    /* linum++; */
    lnptr = NULL;
  }
  /* free(lnptr); */
  fclose(stream);
}

void free_srclns(void)
{
  /* printf("%zu\n", G_srclns_count); */
  for (size_t ln = 0; ln < G_srclns_count; ++ln)
    {
      /* printf("%zu matz\n", ln); */
      free(G_srclns[ln]);}
}

int main()
{
  read_lines("/home/okavango/Work/let/etude.let");
  free_srclns();
  return 0;
}
