/*
if using glib compile with:

gcc -O0 `pkg-config --cflags --libs glib-2.0` -g -Wall -Wextra -std=c11 -pedantic -o /tmp/read read.c

*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #define NDEBUG */
#include <assert.h>
/* #include <glib.h> */



enum __Type {
  NUMBER, INTEGER, FLOAT,
  LAMBDA,
  UNDEFINED
};

char *stringize_type(enum __Type);

#define MAX_TOKLEN 50		/* bytes max token length */
#define TLTOKSTR "TLTOKSTR"

struct token {
  char str[MAX_TOKLEN];	/* token's string */
  int col_start_idx;			/* start index in line (column start index) */
  int col_end_idx;			/* end index in line (column end index) */
  int linum;			/* line number */
  int id;			/* id of this token (tracked globally) */
  int comidx;			/* comment indices: 0 = (, 1 = ) */
  enum __Type type;
  int ival;
  double fval;
};

#define TOKPATT "(;|:|'|\\)|\\(|[[:alnum:]+-=*]+)"

#define COMMOP "("		/* comment opening token */
#define COMMCL ")"		/* comment closing token */

/* naming convention:
   global variables have 2 leading underscores and a Capital letter
*/

int __Tokid = 1;		/* id 0 is reserved for the toplevel
				   token */


char *stringize_type(enum __Type t)
{
  switch (t) {
  case 0: return "unknown";
  case 1: return "integer";
  case 2: return "float";
  case 3: return "lambda";
  default: return "undefined";
  }
}





/* checks if the string s consists only of blanks and/or newline */
int isempty(char *s)
{
  while (*s) {
    /* if char is something other than a blank or a newline, the string
       is regarded as non-empty. */
    if (!isblank(*s) && *s != '\n')
      return 0;
    s++;
  }
  return 1;
}



char **read_lines__Hp(char *path, size_t *count)
{
  FILE *stream;
  stream = fopen(path, "r");
  if (!stream) {
    fprintf(stderr, "can't open source '%s'\n", path);
    exit(EXIT_FAILURE);
  }
  char *lineptr = NULL;
  size_t n = 0;
  char **srclns = NULL;
  while ((getline(&lineptr, &n, stream) != -1)) {
    if (!isempty(lineptr)) {
      /* increment *count first, otherwise realloc will be called with size 0 :-O */
      if ((srclns = realloc(srclns, (*count + 1) * sizeof(char *))) != NULL) {
	*(srclns + (*count)++) = lineptr;
	lineptr = NULL;
      } else exit(EXIT_FAILURE);
    }
  }
  free(lineptr);
  fclose(stream);
  return srclns;
}

void free_lines(char **lines, size_t count)
{
  char **base = lines;
  while (count--) free(*lines++);
  free(base);
}



struct token *tokenize_line__Hp(char *line, size_t *line_toks_count, size_t *all_tokens_count, int linum)
{
  regex_t re;
  int errcode;			
  if ((errcode = regcomp(&re, TOKPATT, REG_EXTENDED))) { /* compilation failed */
    size_t buff_size = regerror(errcode, &re, NULL, 0); /* inspect the required buffer size */
    char buff[buff_size+1];	/* need +1 for the null terminator??? */
    (void)regerror(errcode, &re, buff, buff_size);
    fprintf(stderr, "parse error\n");
    fprintf(stderr, "regcomp failed with: %s\n", buff);
    exit(errcode);
  }
  regmatch_t match[1];	/* interesed only in the whole match */
  int offset = 0, tokstrlen;
  struct token *tokptr = NULL;
  /* overall size of memory allocated for tokens of the line sofar */
  size_t memsize = 0;
  /* int tokscnt = 0; */
  while (!regexec(&re, line + offset, 1, match, REG_NOTBOL)) { /* a match found */
    /* make room for the new token */
    memsize += sizeof(struct token);
    if ((tokptr = realloc(tokptr, memsize)) != NULL) { /* new memory allocated successfully */
      tokstrlen = match[0].rm_eo - match[0].rm_so;
      struct token t;
      memcpy(t.str, line + offset + match[0].rm_so, tokstrlen);
      t.str[tokstrlen] = '\0';
      /* t.numtype = numtype(t.str); */
      /* t.isprim = isprim(t.str); */
      t.id = __Tokid++;
      t.col_start_idx = offset + match[0].rm_so;
      t.col_end_idx = t.col_start_idx + tokstrlen;
      t.linum = linum;
      t.comidx = 0;
      *(tokptr + *line_toks_count) = t;
      (*all_tokens_count)++;
      (*line_toks_count)++;
      offset += match[0].rm_eo;
    } else {
      fprintf(stderr, "realloc failed while tokenizing line %d at token %s", linum, "TOKEN????");
      /* just break out of executaion if haven't enough memory for the
	 next token. leave the freeing & cleanup over for the os! */
      exit(EXIT_FAILURE);
    }
  }
  regfree(&re);  
  return tokptr;
}


struct token *tokenize_source__Hp(char *path, size_t *all_tokens_count)
{
  size_t lines_count = 0;
  char **lines = read_lines__Hp(path, &lines_count);
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line__Hp */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line__Hp(lines[l], &line_toks_count, all_tokens_count, l);
    if ((tokens = realloc(tokens, *all_tokens_count * sizeof(struct token))) != NULL) {
      for (size_t i = 0; i < line_toks_count; i++) {
	*(tokens + i + global_toks_count_cpy) = lntoks[i];
      }
    } else {
      exit(EXIT_FAILURE);
    }
    free(lntoks);
    lntoks=NULL;
  }
  free_lines(lines, lines_count);
  return tokens;
}

struct token *tokenize_lines__Hp(char **srclns, size_t lines_count,
				  size_t *all_tokens_count)
{
  struct token *tokens = NULL;
  struct token *lntoks = NULL;
  size_t line_toks_count, global_toks_count_cpy;
  for (size_t l = 0; l < lines_count; l++) {
    line_toks_count = 0;
    /* take a snapshot of the number of source tokens sofar, before
       it's changed by tokenize_line__Hp */
    global_toks_count_cpy = *all_tokens_count;
    lntoks = tokenize_line__Hp(srclns[l], &line_toks_count, all_tokens_count, l);
    if ((tokens = realloc(tokens, *all_tokens_count * sizeof(struct token))) != NULL) {
      for (size_t i = 0; i < line_toks_count; i++) {
	*(tokens + i + global_toks_count_cpy) = lntoks[i];
      }
    } else {
      exit(EXIT_FAILURE);
    }
    free(lntoks);
    lntoks=NULL;
  }
  return tokens;
}


int iscom_open(struct token tok) {return !strcmp(tok.str, COMMOP);}
int iscom_close(struct token tok) {return !strcmp(tok.str, COMMCL);}

/* comment index 1 is the start of an outer-most comment block. this
   function is the equivalent of set_commidx_ip(toks) in the let.py
   file. */
void index_comments(struct token *tokens, size_t all_tokens_count)
{
  int idx = 1;
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (iscom_open(tokens[i]))
      tokens[i].comidx = idx++;
    else if (iscom_close(tokens[i]))
      tokens[i].comidx = --idx;
  }
}

struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count,
				  size_t all_tokens_count) /* nct = non-comment token */
{
  index_comments(toks, all_tokens_count);
  struct token *nctoks = NULL;	/* non-comment tokens */
  int isincom = false;		/* are we inside of a comment block? */
  for (size_t i = 0; i < all_tokens_count; i++) {
    if (toks[i].comidx == 1) {
      if (isincom) isincom = false;
      else isincom = true;
    } else if (!isincom) {
      /* not in a comment block, allocate space for the new non-comment token */
      /* (*nctok_count)++; */
      if ((nctoks = realloc(nctoks, ++(*nctok_count) * sizeof(struct token))) != NULL)
	/* the index for the new token is one less than the current number of non-comment tokens */
	*(nctoks + *nctok_count - 1) = toks[i];
      else
	exit(EXIT_FAILURE);
    }
  }
  free(toks);
  return nctoks;
}

/* *********************************************************** */

/* int main() */
/* { */
/*   char *strarr[2] = { */
/*     "pret + 2 3", */
/*     "       40 5" */
/*   }; */
/*   size_t all_tokens_count = 0; */
/*   /\* struct token *toks = tokenize_source__Hp("/home/amir/a.let", &all_tokens_count); *\/ */
/*   struct token *toks = tokenize_lines__Hp(strarr, 2, &all_tokens_count); */
/*   size_t nctok_count = 0; */
/*   struct token *nct = remove_comments__Hp(toks, &nctok_count, all_tokens_count); */
/*   printf(">>> %zu\n", all_tokens_count); */
/*   for (size_t i = 0; i<nctok_count;i++) { */
/*     printf("%zu- %s \n", i, nct[i].str); */
/*   } */
/*   free(nct);     */
/*   exit(EXIT_SUCCESS); */
/* } */

/* *********************************************************** */


// alle funktionen mussen ein cell * nehmen und eins zurückgeben!!!


int isdig(char c)
{
  return ('0' <= c) && (c <= '9');
}

enum __Type numtype(char *s)
{
  bool dot = false;
  while (*s) {
    if (!isdig(*s)) {
      /* if a dot: set the type to float, but go on looking the rest
	 (which must be digit only to give a float!) */
      if (*s == '.') {
	dot = true;
	s++;		
      } else return UNDEFINED;	/* if not a digit and not a dot: Not A Number */
    } else s++;			/* if a digit: go on looking the rest */
  }
  return dot ? FLOAT : INTEGER;
}
struct block;

struct cell {
  struct token car;
  struct cell *cdr;
  struct cell *in_block_cdr;
  enum __Type type;
  struct block *emblock;	/* embedding block of this cell */
  struct cell *linker;		/* the cell linking into this cell */
};

char *cellstr(struct cell *c) {return c->car.str;}


/* Type Lambda */
typedef void (*lambda_t)(struct cell *);

struct env;

/* only symbols will have envs!!! */
struct symbol {
  int id;
  char *name;
  lambda_t lambda;		/* has a function value? */
  struct cell *cell;
};

struct symbol add = {
  .id=23,
  .name="ADD",
  .lambda=NULL,
  
};

/* 
defun add x: y: + x y

 */

/* void add(struct cell *args) */
/* { */
/*   struct cell *base = args; */
/*   int i = 0;		/\* ival of + *\/ */
/*   do { */
/*     args = args->cdr; */
/*     eval(args); */
/*     i += args->ival; */
/*   } while (args->cdr != NULL); */
/*   base->ival = i; */
/* } */


/* builtin functions */
char *__Builtins[] = {
  "+", "*", "-", "/"
};
int __Builtins_count = 4;

/* is the cell c a builtin? */
bool isbuiltin(struct cell *c)
{
  for (int i = 0; i < __Builtins_count; i++)
    if (!strcmp(cellstr(c), __Builtins[i]))
      return true;
  return false;
}




/* int __Envid = 0; */

/* struct env { */
/*   int id; */
/*   GHashTable *symht;		/\* hashtable keeping known symbols *\/ */
/*   struct env *parenv;		/\* parent environment *\/ */
/* }; */

/* struct env *make_env__Hp(int id, struct env *parenv) */
/* { */
/*   struct env *e = malloc(sizeof(struct env)); */
/*   e->id = id; */
/*   e->symht = g_hash_table_new(g_str_hash, g_str_equal); */
/*   e->parenv = parenv; */
/*   return e; */
/* } */


/* int __Blockid = 0; */
#define MAX_BLOCK_SIZE 10

struct block {
  int id;
  struct cell cells[MAX_BLOCK_SIZE];
  /* struct env env; */
  int size;			/* number of cells */

  int child_blocks_count;
  struct block **child_blocks;
  struct block *emblock; 	/* embedding block */
};


/* is b directly or indirectly embedding c? */
bool is_embedded_in(struct cell c, struct block b)
{
  return (c.car.col_start_idx > b.cells[0].car.col_start_idx) && (c.car.linum >= b.cells[0].car.linum);
}


/* all blocks in which the cell is embedded. returns a pointer which
   points to pointers to block structures, so it's return value must
   be freed (which doesn't any harm to the actual structure pointers
   it points to!) */
struct block **embedding_blocks__Hp(struct cell c, struct block **blocks,
				   int bcount, int *ebs_count)
{
  struct block **ebs = NULL;
  for (int i = 0; i < bcount; i++)
    if (is_embedded_in(c, *blocks[i])) {
      if ((ebs = realloc(ebs, (*ebs_count + 1) * sizeof(struct block *))) != NULL)
	*(ebs + (*ebs_count)++) = *(blocks + i);
      else exit(EXIT_FAILURE);
    }
  /* for (int i =0; i< *ebs_count;i++) */
  /*   printf("%s %s\n", c.car.str, ebs[i]->cells[0].car.str); */
  return ebs;			/* free(ebs) */
}

/* returns the max line number */
int bottomline(struct block **embedding_blocks, int ebs_count)
{
  int ln = 0;
  for (int i = 0; i < ebs_count; i++) {
    if ((*(embedding_blocks + i))->cells[0].car.linum > ln)
      ln = (*(embedding_blocks + i))->cells[0].car.linum;
  }
  return ln;
}

struct block **bottommost_blocks__Hp(struct block **embedding_blocks,
				     int ebs_count, int *bmbs_count)
{
  int bln = bottomline(embedding_blocks, ebs_count);
  struct block **bmbs = NULL;
  for (int i = 0; i < ebs_count; i++) {
    if ((*(embedding_blocks + i))->cells[0].car.linum == bln) {
      if ((bmbs = realloc(bmbs, (*bmbs_count + 1) * sizeof(struct block *))) != NULL) {
	*(bmbs + (*bmbs_count)++) = *(embedding_blocks + i);
      }	else exit(EXIT_FAILURE);
    }
  }
  /* free the pointer to selected (i.e. embedding) block pointers */
  free(embedding_blocks);
  
  /* for (int i = 0; i<*bmbs_count;i++) */
  /*   printf("%d %d %s\n", *bmbs_count ,bln, bmbs[i]->cells[0].car.str); */
  
  return bmbs;
}

static struct block *rightmost_block(struct block **bottommost_blocks, int bmbs_count)
{
  int col_start_idx = -1;			/* start index */
  struct block *rmb =NULL;
  for (int i = 0; i < bmbs_count; i++) {
    if ((*(bottommost_blocks + i))->cells[0].car.col_start_idx > col_start_idx) {
      /* printf("%d.%d.%d\n", i, col_start_idx, (*(bottommost_blocks + i))->cells[0].car.col_start_idx); */
      /* pick the hitherto rightmost block (note that the address is
	 coming from the original blocks in parse__Hp) */
      rmb = *(bottommost_blocks + i);
      /* col_start_idx = (*(bottommost_blocks + i))->cells[0].car.col_start_idx; */
      col_start_idx = rmb->cells[0].car.col_start_idx;
    }
  }
  /* printf("RMB %p\n", rmb); */
  free(bottommost_blocks);
  return rmb;
}

/* which one of the blocks is the direct embedding block of c? */
struct block *embedding_block(struct cell c, struct block **blocks, int bcount)
{
  int ebs_count = 0;
  struct block **embedding_blocks = embedding_blocks__Hp(c, blocks, bcount, &ebs_count);
  int bmbs_count = 0;
  struct block **bottommost_blocks = bottommost_blocks__Hp(embedding_blocks, ebs_count, &bmbs_count);
  struct block *eb = rightmost_block(bottommost_blocks, bmbs_count);
  /* printf("C %s bmbscount %d\n", c.car.str, bmbs_count); */
  /* for (int i =0;i<bmbs_count;i++) */
  /*   printf("B %p EB: %p | ", bottommost_blocks[i], eb); */
  /* printf("\n"); */
  return eb;
}

/* passing a token pointer to set it's fields */
void guess_token_type(struct token *t)
{
  enum __Type tp;
  if ((tp = (numtype(t->str)))) {
    if (tp == INTEGER) {
      t->ival = atoi(t->str);
      t->type = INTEGER;
    } else if (tp == FLOAT) {
      t->fval = atof(t->str);
      t->type = FLOAT;
    }
  } else {
    t->type = UNDEFINED;
  }
}



/* void eval(struct cell *sexp) */
/* { */
/*   switch (sexp->type) { */
/*   case NUMBER: */
/*     /\* guess_token_type(sexp); *\/ */
/*     printf("type %d %s\n", sexp->type, sexp->car.str); */
/*     break; */
/*   case LAMBDA: */
/*     _addition(sexp); */
/*     break; */
/*   default: */
/*     break; */
/*   } */
/* } */


/* returns a list of linked cells made of tokens */
struct cell *linked_cells__Hp(struct token tokens[], size_t count)
{
  struct cell *prev, *root;	/* store previous and first cell address */
  for (size_t i = 0; i < count; i++) {
    struct cell *c = malloc(sizeof(struct cell));
    if (i == 0) root = c;
    guess_token_type(tokens+i);	/* pass the pointer to the token */
    c->car = tokens[i];
    if (i > 0)
      prev->cdr = c;
    if (i == count-1)
      c->cdr = NULL;
    prev = c;
  }
  return root;
}

/* struct cell *doubly_linked_cells(struct cell *c) */
/* { */
/*   struct cell *root = c;	/\* keep the address of root for return *\/ */
/*   struct cell *curr = NULL;		/\* current cell *\/ */
/*   while (c->cdr) { */
/*     c->linker = curr; */
/*     curr = c; */
/*     c = c->cdr; */
/*   } */
/*   c->linker = curr; */
/*   return root; */
/* } */

void free_linked_cells(struct cell *c)
{
  struct cell *tmp;
  while (c != NULL) {
    tmp = c;
    c = c->cdr;
    free(tmp);
  }
}

struct block __TLBlock = {
  /* id */
  0,
  /* cells */
  {
    {				/* cells[0] */
      /* car token */
      {.str = TLTOKSTR,
       .col_start_idx = -1,
       .col_end_idx = 100,		/* ???????????????????????????????????????? */
       .linum = -1,
       .id = 0
      },
      /* cdr cell pointer */
      NULL,
      /* in block cdr */
      NULL,
      /* type ??? */
      UNDEFINED,
      NULL,
      NULL
    }
  },
  
  /* /\* env (Toplevel Environment) *\/ */
  /* { */
  /*   /\* id *\/ */
  /*   0, */
  /*   /\* syms (GHashtable *), must be populated yet *\/ */
  /*   NULL, */
  /*   /\* parenv *\/ */
  /*   NULL */
  /* }, */
  
  /* size */
  1,
  /* child_blocks_count */
  0,
  /* child_blocks */
  NULL,
  NULL
};




/* 
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./-
*/



struct block **parse__Hp(struct cell *linked_cells_root, int *bcount)
{
  /* this is the blocktracker in the python prototype */
  /* struct block *root = malloc(sizeof(struct block *)); */
  struct block **blocks = malloc(sizeof(struct block *)); /* make room for &__TLBlock */
  *(blocks + (*bcount)++)  = &__TLBlock;
  /* root = &__TLBlock; */
  struct cell *c = linked_cells_root;
  struct block *emblock = NULL;
  int bid = 1;			/* block id */

  while (c) {
    /* find out the direct embedding block of the current cell */
    emblock = embedding_block(*c, blocks, *bcount);
    /* if (emblock == NULL) */
    /*   printf("cell %s emblock %p %p\n", c->car.str, emblock, &__TLBlock); */
    if (isbuiltin(c)) {
      /* printf("* builtin %d %s\n", isbuiltin(c), cellstr(c)); */
      if ((blocks = realloc(blocks, (*bcount + 1) * sizeof(struct block *))) != NULL) {
	struct block *new_block = malloc(sizeof *new_block);
	new_block->id = bid++;
	new_block->cells[0] = *c;
	new_block->size = 1;
	new_block->child_blocks_count = 0;
	new_block->child_blocks = NULL;
	new_block->emblock = emblock;
	
	*(blocks + (*bcount)++) = new_block;
	/* 
	   Wenn emblock null ist, dann ist es ein block dessen HEAD am Anfang einer Zeile steht!
	   Für so einen der emblock ist einfach der TLBlock. Alle andere new_blocks
	   müssen schon einen anderen eigenen emblock haben!
	 */
	if (emblock != NULL) {
	  emblock->child_blocks = realloc(emblock->child_blocks, sizeof(struct block *) * (emblock->child_blocks_count + 1));
	  emblock->child_blocks[emblock->child_blocks_count++] = new_block;	
	} else {		/* dann embedding block ist toplevel block */
	  __TLBlock.child_blocks = realloc(__TLBlock.child_blocks, sizeof(struct block *) * (__TLBlock.child_blocks_count + 1));
	  __TLBlock.child_blocks[__TLBlock.child_blocks_count++] = new_block;
	}
	
      } else exit(EXIT_FAILURE);
      
    } else emblock->cells[emblock->size++] = *c;
    c = c->cdr;
  }
  return blocks;
}

void free_parser_blocks(struct block **blocks, int bcount)
{
  /* the first pointer in **blocks points to the tlblock, thats why we
     can't free *(blocks + 0). that first pointer will be freed after
     the loop. */
  for (int i = 1; i < bcount; i++) free(*(blocks + i));
  free(blocks);
}

/* struct block **eval(struct block **blocks, int blocks_count) */
/* { */
  
/* } */

#define X 2
int main()
{

  char *lines[X] = {
    "+ 3 / 4 20",
    "* 2 3"
  };
  size_t all_tokens_count = 0;
  /* struct token *toks = tokenize_source__Hp("/home/amir/a.let", &all_tokens_count); */
  struct token *toks = tokenize_lines__Hp(lines, X, &all_tokens_count);
  size_t nctok_count = 0;
  struct token *nct = remove_comments__Hp(toks, &nctok_count, all_tokens_count);
  
  /* for (size_t i = 0; i<nctok_count;i++) { */
  /*   printf("TOK-%zu. %s \n", i, nct[i].str); */
  /* } */

  struct cell *c = linked_cells__Hp(nct, nctok_count);
  struct cell *base = c;
  
  int bcount = 0;
  /* struct block **b = parse__Hp(c, &bcount); */
  struct block **b = parse__Hp(c, &bcount);

  for (int i = 0; i <bcount;i++) {
    printf("block id %d, sz %d head[%s] EmblockHead[%s]\n", b[i]->id, b[i]->size, b[i]->cells[0].car.str,
	   b[i]->emblock ? b[i]->emblock->cells[0].car.str : "NULL");
    /* for (int j =0; j<b[i]->child_blocks_count;j++) */
    /*   printf(" ChildBlock: %s\n", b[i]->child_blocks[j]->cells[0].car.str); */
    for (int j = 0; j<b[i]->size;j++)
      printf("  CellStr %s\n", b[i]->cells[j].car.str);
  }
  
  free_parser_blocks(b, bcount);
  
  free_linked_cells(base);
  
  free(nct);
    
  exit(EXIT_SUCCESS);


}

