
#include "let.h"


extern struct letdata *pret(struct letdata *);
extern struct letdata *GJ(void);
extern bool is_define(struct block *);
extern bool is_association(struct cell *);
extern bool is_bound_parameter(struct cell *, struct block *);

char *bound_parameter_name(char *param)
{
  char *name = malloc(strlen(param) - 1);
  for (size_t i = 0; i < (strlen(param) - 2); i++) {
    name[i] = param[i];
  }
  name[strlen(param) - 2] = '\0';
  return name;
}



/* eval evaluiert einen Baum */
struct letdata *eval__dynmem(struct block_item *item,
			 struct env *local_env,
			 struct env *global_env)
{
  struct letdata *result = malloc(sizeof(struct letdata)); /* !!!!!!!!!! FREE!!!!!!!!!!!eval__dynmem */
  switch (item->type) {
  case CELL:
    switch (celltype(item->cell_item)) {
    case INTEGER:
      result->type = INTEGER;
      result->value.dataslot_int = item->cell_item->ival;
      break;
    case FLOAT:
      result->type = FLOAT;
      result->value.dataslot_float = item->cell_item->fval;
      break;
    case SYMBOL:
      /* a symbol not contained in a BIND expression (sondern hängt einfach so rum im text) */
      {
	struct symbol *sym;
	char *symname = item->cell_item->car.str;
	/* struct symbol *sym = g_hash_table_lookup(local_env->hash_table, item->c->car.str); */
	struct env *e = local_env;
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
    break;			/* break CELL */
  case BLOCK:
    if (!strcmp(block_head(item->block_item).car.str, LAMBDA_KW)) {
      result->type = LAMBDA; /* lambda objekte werden nicht in parse time generiert */
      struct lambda *lambda = malloc(sizeof (struct lambda));
      lambda->lambda_env = item->block_item->env->enclosing_env;
      switch (item->block_item->arity) {
      case 0:
	/* wenn arity 0 ist, dann ist das nächste item gleich das return expression */
	/* lambda->return_expr = &(item->block_item->items[1]); */
	lambda->return_expr = item->block_item->items + 1;
	result->value.dataslot_lambda = lambda;
	/* result->value.fn = &GJ; */
	/* result->value.fn = struct letdata *(*)(void); */
	break;
      }
      
    } else if (!strcmp(block_head(item->block_item).car.str, "call")) {
      struct letdata *lambda_name_or_expr = eval__dynmem(&(item->block_item->items[1]), local_env, global_env);
      struct env *lambda_env = lambda_name_or_expr->value.dataslot_lambda->lambda_env;
      /* printf("%s", stringify_type(lambda_name_or_expr->type)); */
      result = eval__dynmem(lambda_name_or_expr->value.dataslot_lambda->return_expr,
			    /* local_env, */
			    lambda_env,
			    global_env);
      
    } else if (!strcmp(block_head(item->block_item).car.str, "pret")) {
      /* struct letdata *thing = eval__dynmem(&((item->block_item)->items[1]), local_env, global_env); */
      /* result->type = thing->type; */
      /* result->value.dataslot_int =thing->value.dataslot_int; */
      /* printf("-> %s\n", stringify_type(result->type)); */
      result = pret(eval__dynmem(&((item->block_item)->items[1]), local_env, global_env));
    } else if (!strcmp(block_head(item->block_item).car.str, "gj")) {
      result = GJ();
      
    } else if (is_define(item->block_item)) { /* is_assignment */
      /* don't let the name of the binding to go through eval! */
      char *define_name = item->block_item->cells[1].car.str; /* name of the definition */
      /* data can be a lambda expr or some constant or other names etc. */
      struct letdata *define_data = eval__dynmem(item->block_item->items + 2,
					     item->block_item->env,
					     /* local_env, */
					     global_env);
      struct symbol *sym= malloc(sizeof (struct symbol));
      sym->symbol_name = define_name;
      sym->symbol_data = define_data;
      /* definitions are always saved in the global environment, no
	 matter in which environment we are currently */
      g_hash_table_insert(global_env->hash_table, define_name, sym);
      result->type = SYMBOL;
      result->value.dataslot_symbol = sym;
      
    } else if (is_association(item->block_item->cells)) { /* = &(item->block_item->cells[0]) */
      /* add let parameters to it's hashtable */
      /* index 0 ist ja let selbst, fangen wir mit 1 an */
      for (int i = 1; i < item->block_item->size - 1;i++) {
	switch (item->block_item->items[i].type) {
	case CELL:		/* muss ein parameter ohne Wert sein, bind to NIL */
	  printf("%s %s\n",item->block_item->items[i].cell_item->car.str,
		 stringify_cell_type(celltype(item->block_item->items[i].cell_item)));
	  break;
	case BLOCK:		/* muss ein bound parameter sein! */
	  {
	    /* So kann ich voraussetzen dass diese Teile alle
	       parameter sind und bis zum letzten Ausruck alles
	       parameter bleibt! */
	    assert(is_bound_parameter(item->block_item->items[i].block_item->cells,
				      item->block_item));
	    int bound_parameter_block_size = item->block_item->items[i].block_item->size;
	    assert(bound_parameter_block_size == 2);
	    char *parameter = item->block_item->items[i].block_item->cells[0].car.str;
	    /* char parameter_name[strlen(parameter)-1]; /\* jajaaaa VLA! *\/ */
	    /* /\* memset(parameter_name, '\0', sizeof (parameter_name)); *\/ */
	    /* strncpy(parameter_name, parameter, strlen(parameter)-2); */
	    /* /\* memcpy(parameter_name, parameter, strlen(parameter)-2); *\/ */
	    /* parameter_name[strlen(parameter)-2]='\0'; */
	    /* char *parameter_name = "x"; */
	    char *param_name = bound_parameter_name(parameter); /* problem mit strncpy */
	    struct letdata *parameter_data = eval__dynmem(&(item->block_item->items[i].block_item->items[1]),
						      item->block_item->env,
						      global_env);
	    struct symbol *symbol = malloc(sizeof (struct symbol));
	    symbol->symbol_name = param_name;
	    symbol->symbol_data = parameter_data;	    
	    g_hash_table_insert(item->block_item->env->hash_table, param_name, symbol);
	    break;
	  }
	  
	  /* { */
	  /*   if (i==item->block_item->size-1){ */
	  /*     result = eval__dynmem(item->block_item->items + i, */
	  /* 			item->block_item->env, */
	  /* 			global_env); */
	  /*     break; */
	  /*   } else { */
	  /*     int bound_parameter_block_size = item->block_item->items[i].block_item->size; */
	  /*     assert(bound_parameter_block_size == 2); */
	  /*     char *parameter = item->block_item->items[i].block_item->cells[0].car.str; */
	  /*     /\* char parameter_name[strlen(parameter)-1]; /\\* jajaaaa VLA! *\\/ *\/ */
	  /*     /\* /\\* memset(parameter_name, '\0', sizeof (parameter_name)); *\\/ *\/ */
	  /*     /\* strncpy(parameter_name, parameter, strlen(parameter)-2); *\/ */
	  /*     /\* /\\* memcpy(parameter_name, parameter, strlen(parameter)-2); *\\/ *\/ */
	  /*     /\* parameter_name[strlen(parameter)-2]='\0'; *\/ */
	  /*     /\* char *parameter_name = "x"; *\/ */
	  /*     char *param_name = bound_parameter_name(parameter); /\* problem mit strncpy *\/ */
	  /*     struct letdata *parameter_data = eval__dynmem(&(item->block_item->items[i].block_item->items[1]), */
	  /* 						item->block_item->env, */
	  /* 						global_env); */
	  /*     struct symbol *symbol = malloc(sizeof (struct symbol)); */
	  /*     symbol->symbol_name = param_name; */
	  /*     symbol->symbol_data = parameter_data; */
	    
	  /*     g_hash_table_insert(item->block_item->env->hash_table, param_name, symbol); */
	  /*     break; */
	  /*   } */
	  /* } */
	  
	}

    
      }
      result = eval__dynmem(item->block_item->items + (item->block_item->size - 1),
			item->block_item->env,
			global_env);
	  

      
    }
    break;			/* break BLOCK */
  default: break;
  }
  return result;
}

struct letdata *global_eval(struct block *root,
			    struct env *local_env,
			    struct env *global_env)
{
  for (int i = 0; i < (root->size - 1); i++) {
    eval__dynmem(&(root->items[i]), local_env, global_env);
  }
  return eval__dynmem(&(root->items[root->size - 1]), local_env, global_env);
}
