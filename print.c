#include <glib.h>
#include <stdio.h>
#include "token.h"
#include "type.h"
#include "unit.h"

#define PROMPT "=> "

void print_indent(int i) {
    int n = 2;
    char s[(i*n)+1];
    for (int k =0; k<i;k++) {
        s[k*n] = '|';
        s[(k*n)+1]=' ';
    }
    s[(i*n)] = '\0';
    printf("%s", s);
}


gboolean
print_node(GNode *node, gpointer data)
{
    print_indent((guint)g_node_depth(node) - 1);
    printf(UNIT_FORMAT,
           ((struct Unit *)node->data)->token.str,
           /* token line starts at 0, add 1 to get the physical
            * line number! */
           ((struct Unit *)node->data)->token.line + 1,
           ((struct Unit *)node->data)->token.col_start_idx,
           ((struct Unit *)node->data)->token.col_end_idx,
           ((unitp_t)node->data)->uuid,
           stringify_type(unit_type((struct Unit *)node->data)),
           (gpointer)node,
           (gpointer)node->data,
           g_node_n_children(node),
           ((struct Unit *)node->data)->is_atomic,
           ((struct Unit *)node->data)->max_cap
        );
    puts("");
    return false;
}

void
print_ast3(GNode *root)
{
    g_node_traverse(root, G_PRE_ORDER, G_TRAVERSE_ALL,
                    -1, (GNodeTraverseFunc)print_node, NULL);
}


void
print_lambda(struct Arv_data *lambda_data)
{
    printf("%s(%p) ", LAMBDA_KW, (void *)lambda_data);
}
void
print_int(struct Arv_data *int_data)
{
    printf("%d ", int_data->slots.tila_int);
}
void
print_float(struct Arv_data *float_data)
{
    printf("%f ", float_data->slots.tila_float);
}
void
print_bool(struct Arv_data *bool_data)
{
    printf("%s ", bool_data->slots.tila_bool ? TRUEKW : FALSEKW);
}

void
print_data(struct Arv_data *);

void
print_list(struct Arv_data *list_data)
{
    printf("%s:%d(%p) ", LIST_OP_KW, list_data->slots.arv_list->size, (void *)list_data);
    /* use a copy of items, since the print_list is also used in other
     * places (e.g. eval_show_op), otherwise the item pointer is fully
     * consumed when its needed for the print(eval...) */
    GList *itemcp = g_list_copy(list_data->slots.arv_list->item);
    while (itemcp) {
        print_data(itemcp->data);
        itemcp = itemcp->next;
    }
}


/* string representation of data, this is the P in REPL */
/* data arg is the evaluated expression (is gone through eval already) */
void
print_data(struct Arv_data *data)
{
    switch (data->type) {
    case INT: print_int(data); break;
    case FLOAT: print_float(data); break;
    case LAMBDA: print_lambda(data); break;
    case BOOL: print_bool(data); break;
    case LIST: print_list(data); break;
    default: break;
    }
}

void
print(struct Arv_data *data)
{
    /* printf("%s", PROMPT); */
    print_data(data);
    printf("\n");
}
