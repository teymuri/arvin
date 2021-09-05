#include <stdlib.h>
#include <stdio.h>
#include "parser.h"


int main()
{

  /* char *s = " let it  b(e89)"; */
  /* int n =tokenize_line(s, TOKPATT, 0); */

  /* struct token *t = tokenize_line2("/home/okavango/Work/let/etude.let"); */
  (void)tokenize_src("/home/okavango/Work/let/etude.let");
  printf("%d lines\n", number_of_lines);
  /* for (int i = 0; i < n; i++, t++) { */
  /*   /\* printf("** %s s:%d e:%d **\n", line_toks[i].str, line_toks[i].so, line_toks[i].eo); *\/ */
  /*   printf("%d- %s s:%d, e:%d -\n", i, t->str, t->so, t->eo); */
  /* } */
  /* printf("=========\n"); */
  /* free(t-n); */
  /* printf("=========\n"); */
  for (int i = 0; i < number_of_lines; i++){
    printf("%d toks: ", src_toks[i].toks_count);
    for (int j = 0; j < src_toks[i].toks_count; j++){
      printf("'%s' os:%d oe:%d| ", src_toks[i].toks->str,
	     src_toks[i].toks->so, src_toks[i].toks->eo);
      src_toks[i].toks++;
    }
    src_toks[i].toks -= src_toks[i].toks_count; /* put back for freeing */
    printf("\n");
  }
  free_parser();			/* free_parser */
  /* size_t n = read_lines("/home/okavango/Work/let/etude.let"); */
  /* for (size_t i = 0; i<n; i++) */
  /*   printf("%zu %s", i, srclns[i]); */
  /* free_srclns(n); */
  exit(EXIT_SUCCESS);
}
