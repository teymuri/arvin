#ifndef LET_READ_H
#define LET_READ_H

struct token *tokenize_source__Hp(char *path, size_t *all_tokens_count);
struct token *remove_comments__Hp(struct token *toks, size_t *nctok_count,
				  size_t all_tokens_count);
#endif	/* LET_READ_H */
