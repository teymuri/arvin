#ifndef LET_EVAL_H
#define LET_EVAL_H

struct LetData *global_eval(struct Bundle *root,
			    struct Env *local_env,
			    struct Env *global_env);

#endif	/* LET_EVAL_H */
