#ifndef ARV_PRINT_H
#define ARV_PRINT_H


void print(struct Arv_data *);
void print_ast3(GNode *);
gboolean print_node(GNode *, gpointer);

void print_list(struct Arv_data *);
void print_bool(struct Arv_data *);
void print_float(struct Arv_data *);
void print_int(struct Arv_data *);
void print_lambda(struct Arv_data *);

#endif	/* ARV_PRINT_H */
