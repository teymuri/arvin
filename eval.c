#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include "type.h"
#include "let_data.h"
#include "token.h"
#include "env.h"
#include "atom.h"
#include "cons.h"
/* #include "const_item.h" */
#include "core.h"
#include "symbol.h"
#include "lambda.h"


/* is external, defined in ast.c */
bool is_bound_parameter(struct Atom *, struct Cons *);
bool is_bound_binding(struct Atom *);
void amend_lambda_semantics(struct Cons *);

bool is_association(struct Atom *);

char *bound_parameter_name(char *param)
{
  char *name = malloc(strlen(param) - 1);
  for (size_t i = 0; i < (strlen(param) - 2); i++) {
    name[i] = param[i];
  }
  name[strlen(param) - 2] = '\0';
  return name;
}

bool is_define(struct Cons *b)
{
  return !strcmp(b->bricks[0]->token.str, ASSIGNMENT_KEYWORD);
}


/* eval evaluiert einen Baum */
struct Let_data *eval_const_item(struct Cons_item *item,
			 struct Env *local_env,
			 struct Env *global_env)
{
  struct Let_data *result = malloc(sizeof(struct Let_data)); /* !!!!!!!!!! FREE!!!!!!!!!!!eval_const_item */
  switch (item->type) {
  case ATOM:
    switch (atom_type(item->the_unit)) {
    case INTEGER:
      result->type = INTEGER;
      result->value.dataslot_int = item->the_unit->ival;
      break;
    case FLOAT:
      result->type = FLOAT;
      result->value.dataslot_float = item->the_unit->fval;
      break;
    case SYMBOL:
      /* a symbol not contained in a BIND expression (sondern hängt einfach so rum im text) */
      {
	struct Symbol *sym;
	char *symname = item->the_unit->token.str;
	/* struct Symbol *sym = g_hash_table_lookup(local_env->hash_table, item->c->token.str); */
	struct Env *e = local_env;
	while (e) {
	  if ((sym = g_hash_table_lookup(e->hash_table, symname))) {
	    result->type = sym->symbol_data->type;
	    switch (result->type) {
	    case INTEGER:
	      result->value.dataslot_int = sym->symbol_data->value.dataslot_int; break;
	    case FLOAT:
	      result->value.dataslot_float = sym->symbol_data->value.dataslot_float; break;
	    case LAMBDA:
	      result->value.dataslot_lambda = sym->symbol_data->value.dataslot_lambda; break;
	    default: break;
	    }
	    break;
	  } else {
	    e = e->enclosing_env;
	  }
	}
	if (!e) {		/* wir sind schon beim parent von global env angekommen */
	  fprintf(stderr, "unbound '%s'\n", symname);
	  exit(EXIT_FAILURE);
	}
      }
      break;
    default: break;
    }
    break;			/* break ATOM */
  case CONS:
    /* if (is_lambda_unit(&block_head(item->the_const))) { */
    if (!strcmp(block_head(item->the_const).token.str, LAMBDA_KW)) {

      /* int lambda_IS_size = item->the_const->size - 1; /\* head is lambda word itself, cut it off *\/ */
      /* int lambda_SHOULDBE_size = item->the_const->arity + 1; */
      /* printf("%d %d\n", lambda_IS_size, lambda_SHOULDBE_size); */
      /* assert(lambda_IS_size == lambda_SHOULDBE_size); */
      result->type = LAMBDA; /* lambda objekte werden nicht in parse time generiert */
      struct Lambda *lambda = malloc(sizeof (struct Lambda));
      lambda->lambda_env = item->the_const->env->enclosing_env;

      /* item->the_const->elts + 0 ist ja lambda wort selbst!!! */
      switch (item->the_const->arity) {
      case 0:
	/* wenn arity 0 ist, dann ist das nächste item gleich das return expression */
	/* lambda->return_expr = &(item->the_const->elts[1]); */
	lambda->return_expr = item->the_const->elts + 1;
	result->value.dataslot_lambda = lambda;
	/* result->value.fn = &GJ; */
	/* result->value.fn = struct Let_data *(*)(void); */
	break;
      }
      
    } else if (!strcmp(block_head(item->the_const).token.str, "call")) {
      struct Let_data *lambda_name_or_expr = eval_const_item(&(item->the_const->elts[1]), local_env, global_env);
      struct Env *lambda_env = lambda_name_or_expr->value.dataslot_lambda->lambda_env;
      /* printf("%s", stringify_type(lambda_name_or_expr->type)); */
      result = eval_const_item(lambda_name_or_expr->value.dataslot_lambda->return_expr,
			    /* local_env, */
			    lambda_env,
			    global_env);
      
    } else if (!strcmp(block_head(item->the_const).token.str, "pret")) {
      /* struct Let_data *thing = eval_const_item(&((item->the_const)->elts[1]), local_env, global_env); */
      /* result->type = thing->type; */
      /* result->value.dataslot_int =thing->value.dataslot_int; */
      /* printf("-> %s\n", stringify_type(result->type)); */
      result = pret(eval_const_item(&((item->the_const)->elts[1]), local_env, global_env));
    } else if (!strcmp(block_head(item->the_const).token.str, "gj")) {
      result = GJ();
      
    } else if (is_define(item->the_const)) { /* is_assignment */
      /* don't let the name of the binding to go through eval! */
      char *define_name = item->the_const->bricks[1]->token.str; /* name of the definition */
      /* data can be a lambda expr or some constant or other names etc. */
      struct Let_data *define_data = eval_const_item(item->the_const->elts + 2,
					     item->the_const->env,
					     /* local_env, */
					     global_env);
      struct Symbol *sym= malloc(sizeof (struct Symbol));
      sym->symbol_name = define_name;
      sym->symbol_data = define_data;
      /* definitions are always saved in the global environment, no
	 matter in which environment we are currently */
      g_hash_table_insert(global_env->hash_table, define_name, sym);
      result->type = SYMBOL;
      result->value.dataslot_symbol = sym;
      
    } else if (is_association(*item->the_const->bricks)) { /* = &(item->the_const->bricks[0]) */
      /* add let parameters to it's hashtable */
      /* index 0 ist ja let selbst, fangen wir mit 1 an */
      for (int i = 1; i < item->the_const->size - 1;i++) {
	switch (item->the_const->elts[i].type) {
	case ATOM:		/* muss ein parameter ohne Wert sein, bind to NIL */
	  printf("in Eval bundle Unit BIT: %s %s\n",item->the_const->elts[i].the_unit->token.str,
		 stringify_type(atom_type(item->the_const->elts[i].the_unit)));
	  break;
	case CONS:		/* muss ein bound parameter sein! */
	  {
	    /* So kann ich voraussetzen dass diese Teile alle
	       parameter sind und bis zum letzten Ausruck alles
	       parameter bleibt! */
	    assert(is_bound_binding(*item->the_const->elts[i].the_const->bricks));
	    /* assert(is_bound_parameter(*item->the_const->elts[i].the_const->bricks, item->the_const)); */
	    int bound_parameter_block_size = item->the_const->elts[i].the_const->size;
	    assert(bound_parameter_block_size == 2);
	    char *parameter = item->the_const->elts[i].the_const->bricks[0]->token.str;

	    /* .x */
	    char *param_name=malloc(strlen(parameter)); /* jajaaaa VLA! */
	    strncpy(param_name, parameter + 1, strlen(parameter));
	    /* memcpy(param_name, parameter, strlen(parameter)-2); */
	    /* param_name[strlen(parameter)-2]='\0'; */
	    /* char *param_name = "x"; */
	    
	    /* char *param_name = bound_parameter_name(parameter); /\* problem mit strncpy *\/ */
	    
	    struct Let_data *parameter_data = eval_const_item(&(item->the_const->elts[i].the_const->elts[1]),
						      item->the_const->env,
						      global_env);
	    struct Symbol *symbol = malloc(sizeof (struct Symbol));
	    symbol->symbol_name = param_name;
	    symbol->symbol_data = parameter_data;	    
	    g_hash_table_insert(item->the_const->env->hash_table, param_name, symbol);
	    /* printf("%s %d\n", param_name, g_hash_table_contains(item->the_const->env->hash_table, param_name)); */
	    break;
	  }
	}

    
      }
      result = eval_const_item(item->the_const->elts + (item->the_const->size - 1),
			item->the_const->env,
			global_env);
	  

      
    }
    break;			/* break CONS */
  default: break;
  }
  return result;
}

struct Let_data *eval(struct Cons *root,
		      struct Env *local_env,
		      struct Env *global_env)
{
  /* amend_lambda_semantics(root); */
  for (int i = 0; i < (root->size - 1); i++) {
    eval_const_item(&(root->elts[i]), local_env, global_env);
  }
  return eval_const_item(&(root->elts[root->size - 1]), local_env, global_env);
}
