#ifndef LET_READ_H
#define LET_READ_H

struct Token *tokenize_source__Hp(char *path, size_t *all_tokens_count);
struct Token *polish_tokens(struct Token *toks, size_t *nctok_count,
                            size_t all_tokens_count);
struct Token *tokenize_lines__Hp(char **srclns, size_t lines_count,
                                 size_t *all_tokens_count);
struct Token *tokenize_line__Hp
(char *lnstr, size_t *line_toks_count, size_t *all_tokens_count, int ln);
#endif	/* LET_READ_H */
