#ifndef LET_READ_H
#define LET_READ_H

struct Token **tokenize_source(char *, size_t *);
struct Token *tokenize_source2(char *, size_t *);
struct Token **remove_comments(struct Token *, size_t *, size_t);
struct Token *tokenize_lines__Hp(char **srclns, size_t lines_count,
                                 size_t *all_tokens_count);
struct Token *tokenize_line__Hp
(char *lnstr, size_t *line_toks_count, size_t *all_tokens_count, int ln);

void free_token(struct Token *);

GList *tokenize_src(char *);
void rm_comments2(GList **);

#endif	/* LET_READ_H */
