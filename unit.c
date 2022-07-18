#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include "token.h"
#include "type.h"
#include "unit.h"
#include "read.h"


/* why is this any good??? */
enum Type unit_type(struct Unit *u) {
    switch (u->type) {
    case INT: return INT;
    case FLOAT: return FLOAT;
    case NAME: return NAME;
    case LAMBDA: return LAMBDA;
    case BINDING: return BINDING;
    case BOUND_BINDING: return BOUND_BINDING;
    case PACK_BINDING: return PACK_BINDING;
    case BOUND_PACK_BINDING: return BOUND_PACK_BINDING;
    case PACK: return PACK;
    case BOOL: return BOOL;     /* 11 */
    case LIST: return LIST;     /* 12 */
    case UNIADIC_LAMBDA: return UNIADIC_LAMBDA;
    case VARIADIC_LAMBDA: return VARIADIC_LAMBDA;
        
    case MAND_PARAM: return MAND_PARAM;           /* 15 */
    case OPT_PARAM: return OPT_PARAM;
    case REST_MAND_PARAM: return REST_MAND_PARAM;
    case REST_OPT_PARAM: return REST_OPT_PARAM;
    case CALL_OPT_REST_PARAM: return CALL_OPT_REST_PARAM;
    default: return UNDEFINED;
    }
}

void set_unit_type(struct Unit *c) {
    switch (c->token->type) {
    case INT: c->type = INT; break;
    case FLOAT: c->type = FLOAT; break;
    case NAME: c->type = NAME; break;
    default: break;
    }
}

void set_unit_value(struct Unit *c)
{
    switch (c->token->type) {
    case INT:
        c->ival = atoi(c->token->string);
        break;
    case FLOAT:
        c->fval = atof(c->token->string);
        break;
    default: break;
    }
}

void
free_unit(struct Unit *unit_ptr)
{
    free_token(unit_ptr->token);
    free(unit_ptr);
    unit_ptr = NULL;
}

GList *
unit_list(struct Token **code_tokens,
          size_t code_tokens_count)
{
    GList *list = NULL; /* the returned singly linked list */
    /* struct Unit *unit = NULL; */
    for (size_t i = 0; i < code_tokens_count; i++) {
        struct Unit *unit = malloc(sizeof (struct Unit));
        *unit = (struct Unit){
            .env = NULL,
            .uuid = i + 1,
            .token = code_tokens[i],
            .toklen = code_tokens[i]->string_size, /* not needed really if i've the token already!!! */
            .max_cap = -1,
            
        };
        /* unit = g_new(struct Unit, 1); */
        /* unit->env = NULL; */
        /* unit->lambda_env = g_hash_table_new(g_str_hash, g_str_equal); */
        /* uuid 0 is reserved for the toplevel unit, so start with 1 */
        
        /* unit->uuid = i + 1; */
        /* unit->token = code_tokens[i]; */
        
        /* unit->toklen = strlen(unit->token->string); */

        /* do i need this toklen??? just get unit->token->string_size; */
        
        /* unit->toklen = unit->token->string_size; */
        
        /* maximum absorption of -1 means undefined, i.e. will capture
           ALL coming units! This will be reset later in parser to
           possinly different unsigned integers for different types
           (e.g. parameter bindings will get 1 etc.)*/
        
        /* unit->max_cap = -1; */
        set_unit_value(unit);
        set_unit_type(unit);
        list = g_list_prepend(list, unit);
    }
    list = g_list_reverse(list);
    free(code_tokens);
    return list;
}

GList *
unit_list2(GList **src_tok_list)
{
    GList *list = NULL; /* the returned singly linked list */
    size_t i = 1;
    for (GList *link = *src_tok_list;
         link != NULL;
         link = link->next) {
        struct Unit *unit = malloc(sizeof (struct Unit));
        *unit = (struct Unit){
            .env = NULL,
            .uuid = i++,
            .token = link->data,
            .toklen = ((struct Token *)link->data)->string_size,
            .max_cap = -1
        };
        set_unit_value(unit);
        set_unit_type(unit);
        list = g_list_append(list, unit);
    }
    /* Free the list but not it's data (tokens), as these are now
     * transferred into the created units
     * (https://gitlab.gnome.org/GNOME/glib/-/blob/main/glib/glist.c#L183). */
    g_list_free(*src_tok_list);
    return list;
}
