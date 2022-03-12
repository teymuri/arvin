#include <glib.h>
#include <stdio.h>
#include "token.h"
#include "type.h"
#include "unit.h"

#define PROMPT "=>"

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
print_lambda(struct Tila_data *data)
{
    printf(" <Lambda %p>", (void *)data);
}
/* string representation of data, this is the P in REPL */
/* data arg is the evaluated expression (is gone through eval already) */
void
print_data(struct Tila_data *data)
{
    switch (data->type) {
    case INT: printf(" %d", data->slots.tila_int); break;
    case FLOAT: printf(" %f", data->slots.tila_float); break;
    case LAMBDA: print_lambda(data); break;
    case BOOL:
        printf(" %s", data->slots.tila_bool ? TRUEKW : FALSEKW); break;
    case LIST:
        printf(" L#%d", data->slots.tila_list->size);
        while (data->slots.tila_list->item) {
            switch (((struct Tila_data *)data->slots.tila_list->item->data)->type) {
            case INT: printf(" %d", ((struct Tila_data *)data->slots.tila_list->item->data)->slots.tila_int); break;
            case FLOAT: printf(" %f", data->slots.tila_float); break;
            case LAMBDA: print_lambda((struct Tila_data *)data->slots.tila_list->item->data); break;
            case LIST: print_data(data->slots.tila_list->item->data); break;
            default:
                fprintf(stderr, "Can't print\n");
                exit(EXIT_FAILURE);
                break;
            }
            data->slots.tila_list->item = data->slots.tila_list->item->next;
        }
        break;
    default: break;
    }
}

void
print(struct Tila_data *data)
{
    printf("%s", PROMPT);
    print_data(data);
    puts("");
}
